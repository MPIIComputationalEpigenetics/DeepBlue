//
//  genes.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 09.09.2015
//  Copyright (c) 2016 Max Planck Institute for Informatics. All rights reserved.

//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.

//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.

//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//


#include <string>

#include <mongo/bson/bson.h>

#include "../connection/connection.hpp"

#include "../datatypes/metadata.hpp"
#include "../dba/helpers.hpp"

#include "../parser/gtf.hpp"

#include "../extras/utils.hpp"

#include "annotations.hpp"
#include "collections.hpp"
#include "full_text.hpp"
#include "genes.hpp"
#include "helpers.hpp"
#include "remove.hpp"
#include "users.hpp"

#include "../errors.hpp"

namespace epidb {
  namespace dba {
    namespace genes {

      bool build_upload_info(const std::string &user_key, const std::string &client_address, const std::string &content_format,
                             mongo::BSONObj &upload_info, std::string &msg)
      {
        utils::IdName user;
        if (!users::get_user(user_key, user, msg)) {
          return false;
        }

        mongo::BSONObjBuilder upload_info_builder;

        upload_info_builder.append("user", user.id);
        upload_info_builder.append("content_format", content_format);
        upload_info_builder.append("done", false);
        upload_info_builder.append("client_address", client_address);
        time_t time_;
        time(&time_);
        upload_info_builder.appendTimeT("upload_start", time_);

        upload_info = upload_info_builder.obj();

        return true;
      }

      bool build_metadata(const std::string &name, const std::string &norm_name,
                          const std::string &description, const std::string &norm_description,
                          const std::string &format,
                          const mongo::BSONObj& extra_metadata_obj,
                          const std::string &user_key, const std::string &ip,
                          int &dataset_id,
                          std::string &gene_model_id,
                          mongo::BSONObj &gene_model_metadata,
                          std::string &msg)
      {
        if (!helpers::get_increment_counter("datasets", dataset_id, msg) ||
            !helpers::notify_change_occurred("datasets", msg))  {
          return false;
        }

        int _id;
        if (!helpers::get_increment_counter(Collections::GENE_MODELS(), _id, msg) ||
            !helpers::notify_change_occurred(Collections::GENE_MODELS(), msg))  {
          return false;
        }
        gene_model_id = "gs" + utils::integer_to_string(_id);

        mongo::BSONObjBuilder gene_model_metadata_builder;
        gene_model_metadata_builder.append("_id", gene_model_id);
        gene_model_metadata_builder.append(KeyMapper::DATASET(), dataset_id);
        gene_model_metadata_builder.append("name", name);
        gene_model_metadata_builder.append("norm_name", norm_name);
        gene_model_metadata_builder.append("format", format);
        gene_model_metadata_builder.append("description", description);
        gene_model_metadata_builder.append("norm_description", norm_description);
        gene_model_metadata_builder.append("extra_metadata", extra_metadata_obj);

        gene_model_metadata = gene_model_metadata_builder.obj();
        return true;
      }

      bool update_upload_info(const std::string &collection, const std::string &annotation_id,
                              const size_t total_size, const size_t total_genes, std::string &msg)
      {
        Connection c;
        c->update(helpers::collection_name(collection), BSON("_id" << annotation_id),
                  BSON("$set" << BSON("upload_info.total_genes" << (long long) total_genes << "upload_info.total_size" << (long long) total_size << "upload_info.done" << true << "upload_info.upload_end" << mongo::DATENOW)), false, true);

        if (!c->getLastError().empty()) {
          msg = c->getLastError();
          c.done();
          return false;
        }
        c.done();

        return true;
      }

      mongo::BSONObj to_bson(const int dataset_id, const std::string& gene_model_id, const std::string& gene_id, const parser::GTFRow& row)
      {
        mongo::BSONObjBuilder bob;

        bob.append("_id", gene_id);
        bob.append(KeyMapper::DATASET(), dataset_id);
        bob.append(KeyMapper::CHROMOSOME(), row.seqname());
        bob.append(KeyMapper::SOURCE(), row.source());
        bob.append(KeyMapper::FEATURE(), row.feature());
        bob.append(KeyMapper::START(), row.start());
        bob.append(KeyMapper::END(), row.end());
        bob.append(KeyMapper::SCORE(), row.score());
        bob.append(KeyMapper::STRAND(), (std::string) row.strand());
        bob.append(KeyMapper::FRAME(), (std::string) row.frame());

        const parser::GTFRow::Attributes& attributes = row.attributes();
        mongo::BSONObjBuilder attributes_bob;
        for (const auto& kv : attributes) {
          attributes_bob.append(kv.first, kv.second);
        }

        bob.append(KeyMapper::ATTRIBUTES(), attributes_bob.obj());

        // TODO: include in full text

        return bob.obj();
      }


      bool insert(const std::string &name, const std::string &norm_name,
                  const std::string &description, const std::string &norm_description,
                  datatypes::Metadata extra_metadata,
                  const parser::GTFPtr &gtf,
                  const std::string &user_key, const std::string &ip,
                  std::string &gene_model_id, std::string &msg)
      {
        mongo::BSONObj gene_model_metadata;
        mongo::BSONObj extra_metadata_obj = datatypes::metadata_to_bson(extra_metadata);
        int dataset_id;

        if (!build_metadata(name, norm_name, description, norm_description, "GTF", extra_metadata_obj,
                            user_key, ip, dataset_id, gene_model_id, gene_model_metadata, msg)) {
          return false;
        }

        mongo::BSONObj upload_info;
        if (!build_upload_info(user_key, ip, "GTF", upload_info, msg)) {
          return false;
        }
        mongo::BSONObjBuilder gene_model_builder;
        gene_model_builder.appendElements(gene_model_metadata);
        gene_model_builder.append("upload_info", upload_info);

        mongo::BSONObj e = gene_model_builder.obj();
        Connection c;
        c->insert(helpers::collection_name(Collections::GENE_MODELS()), e);
        if (!c->getLastError().empty()) {
          msg = c->getLastError();
          c.done();
          return false;
        }

        if (!search::insert_full_text(Collections::GENE_MODELS(), gene_model_id, gene_model_metadata, msg)) {
          c.done();
          std::string new_msg;
          if (!remove::gene_model(gene_model_id, user_key, new_msg)) {
            msg = msg + " " + new_msg;
          }
          return false;
        }

        size_t total_size = 0;
        size_t total_genes = 0;

        for (const auto& row :  gtf->rows()) {

          int _id;
          if (!helpers::get_increment_counter("genes", _id, msg) ||
              !helpers::notify_change_occurred(Collections::GENES(), msg))  {
            return false;
          }

          std::string gene_id = "gn" + utils::integer_to_string(_id);
          mongo::BSONObj row_obj = to_bson(dataset_id, gene_model_id, gene_id, row);

          c->insert(helpers::collection_name(Collections::GENES()), row_obj);
          if (!c->getLastError().empty()) {
            msg = c->getLastError();
            c.done();
            return false;
          }

          total_genes++;
        }

        if (!update_upload_info(Collections::GENE_MODELS(), gene_model_id, total_size, total_genes, msg)) {
          std::string new_msg;
          if (!remove::gene_model(gene_model_id, user_key, new_msg)) {
            msg = msg + " " + new_msg;
          }
          return false;
        }

        c.done();
        return true;
      }


      bool build_expression_metadata(const std::string &sample_id, const int replica,
                                     const std::string &format,
                                     const mongo::BSONObj& extra_metadata_obj,
                                     const std::string &user_key, const std::string &ip,
                                     int &dataset_id,
                                     std::string &gene_model_id,
                                     mongo::BSONObj &gene_model_metadata,
                                     std::string &msg)
      {
        if (!helpers::get_increment_counter("datasets", dataset_id, msg) ||
            !helpers::notify_change_occurred("datasets", msg))  {
          return false;
        }

        int _id;
        if (!helpers::get_increment_counter(Collections::GENE_EXPRESSIONS(), _id, msg) ||
            !helpers::notify_change_occurred(Collections::GENE_EXPRESSIONS(), msg))  {
          return false;
        }
        gene_model_id = "gx" + utils::integer_to_string(_id);

        mongo::BSONObjBuilder gene_model_metadata_builder;
        gene_model_metadata_builder.append("_id", gene_model_id);
        gene_model_metadata_builder.append(KeyMapper::DATASET(), dataset_id);
        gene_model_metadata_builder.append("sample_id", sample_id);
        gene_model_metadata_builder.append("replica", replica);
        gene_model_metadata_builder.append("format", format);
        gene_model_metadata_builder.append("extra_metadata", extra_metadata_obj);

        gene_model_metadata = gene_model_metadata_builder.obj();
        return true;
      }

      mongo::BSONObj to_bson(const int dataset_id, const std::string& gene_model_id, const std::string& gene_id, const parser::FPKMRow& row)
      {
        mongo::BSONObjBuilder bob;

        bob.append("_id", gene_id);
        bob.append(KeyMapper::DATASET(), dataset_id);
        bob.append(KeyMapper::TRACKING_ID(), row.tracking_id());
        bob.append(KeyMapper::GENE_ID(), row.gene_id());
        bob.append(KeyMapper::GENE_SHORT_NAME(), row.gene_short_name());
        bob.append(KeyMapper::FPKM(), row.fpkm());
        bob.append(KeyMapper::FPKM_LO(), row.fpkm_lo());
        bob.append(KeyMapper::FPKM_HI(), row.fpkm_hi());
        bob.append(KeyMapper::FPKM_STATUS(), row.fpkm_status());

        return bob.obj();
      }

      bool insert_expression(const std::string& sample_id, const int replica, datatypes::Metadata extra_metadata,
                             const parser::FPKMPtr &fpkm,  const std::string &user_key, const std::string &ip,
                             std::string &gene_expression_id, std::string &msg)
      {
        mongo::BSONObj gene_expression_metadata;
        mongo::BSONObj extra_metadata_obj = datatypes::metadata_to_bson(extra_metadata);
        int dataset_id;

        if (!build_expression_metadata(sample_id, replica, "FPKM", extra_metadata_obj,
                                       user_key, ip, dataset_id, gene_expression_id, gene_expression_metadata, msg)) {
          return false;
        }

        mongo::BSONObj upload_info;
        if (!build_upload_info(user_key, ip, "FPKM", upload_info, msg)) {
          return false;
        }
        mongo::BSONObjBuilder gene_expression_builder;
        gene_expression_builder.appendElements(gene_expression_metadata);
        gene_expression_builder.append("upload_info", upload_info);

        mongo::BSONObj e = gene_expression_builder.obj();
        Connection c;
        c->insert(helpers::collection_name(Collections::GENE_EXPRESSIONS()), e);
        if (!c->getLastError().empty()) {
          msg = c->getLastError();
          c.done();
          return false;
        }

        if (!search::insert_full_text(Collections::GENE_EXPRESSIONS(), gene_expression_id, gene_expression_metadata, msg)) {
          c.done();
          std::string new_msg;
          if (!remove::gene_expression(gene_expression_id, user_key, new_msg)) {
            msg = msg + " " + new_msg;
          }
          return false;
        }

        size_t total_size = 0;
        size_t total_genes = 0;

        for (const auto& row :  fpkm->rows()) {

          int _id;
          if (!helpers::get_increment_counter("gene_single_expressions", _id, msg) ||
              !helpers::notify_change_occurred(Collections::GENE_SINGLE_EXPRESSIONS(), msg))  {
            return false;
          }

          std::string gene_id = "gx" + utils::integer_to_string(_id);
          mongo::BSONObj row_obj = to_bson(dataset_id, gene_expression_id, gene_id, row);

          c->insert(helpers::collection_name(Collections::GENE_SINGLE_EXPRESSIONS()), row_obj);
          if (!c->getLastError().empty()) {
            msg = c->getLastError();
            c.done();
            return false;
          }

          total_genes++;
        }

        if (!update_upload_info(Collections::GENE_EXPRESSIONS(), gene_expression_id, total_size, total_genes, msg)) {
          std::string new_msg;
          if (!remove::gene_model(gene_expression_id, user_key, new_msg)) {
            msg = msg + " " + new_msg;
          }
          return false;
        }

        c.done();
        return true;
      }

      bool build_genes_database_query(const std::vector<std::string> &chromosomes, const int start, const int end,
                                      const std::vector<std::string>& genes, const std::vector<std::string>& norm_gene_models,
                                      const bool exactly,
                                      mongo::Query& query, std::string& msg)
      {

        Connection c;
        mongo::BSONObj gene_model_obj = c->findOne(dba::helpers::collection_name(dba::Collections::GENE_MODELS()),
                                        BSON("norm_name" << BSON("$in" << utils::build_array(norm_gene_models))));
        c.done();

        if (gene_model_obj.isEmpty()) {
          msg = "gene model " + utils::vector_to_string<std::string>(norm_gene_models) + " does not exists";
          return false;
        }

        mongo::BSONObjBuilder bob;
        auto dataset_id = gene_model_obj[KeyMapper::DATASET()].Int();
        bob.append(KeyMapper::DATASET(), dataset_id);

        if (!genes.empty()) {
          mongo::BSONArray genes_array = utils::build_regex_array(genes);
          mongo::BSONArray genes_array_2 = genes_array;

          mongo::BSONObj b_in_gene_name = BSON((KeyMapper::ATTRIBUTES() + ".gene_name") << BSON("$in" << genes_array));
          mongo::BSONObj b_in_gene_id = BSON((KeyMapper::ATTRIBUTES() + ".gene_id") << BSON("$in" << genes_array_2));
          bob.append("$or", BSON_ARRAY(b_in_gene_name << b_in_gene_id));
        }

        if (!chromosomes.empty()) {
          bob.append(KeyMapper::CHROMOSOME(), BSON("$in" << utils::build_array(chromosomes)));
        }

        if (exactly) {
          if (start > -1) {
            bob.append(KeyMapper::START(), start);
          }
          if (end > -1) {
            bob.append(KeyMapper::END(), end);
          }
        } else {
          if (start > -1) {
            bob.append(KeyMapper::END(), BSON("$gte" << start));
          }
          if (end > -1) {
            bob.append(KeyMapper::START(), BSON("$lte" << end));
          }
        }

        mongo::BSONObj filter = bob.obj();

        query = mongo::Query(filter).sort(BSON(KeyMapper::CHROMOSOME() << 1 << KeyMapper::START() << 1));

        return true;
      }

      bool get_gene_attribute(const std::string& chromosome, const Position start, const Position end,
                              const std::string& attribute_name, const std::string& gene_model,
                              std::string& attibute_value, std::string& msg)
      {
        mongo::BSONObj gene;
        if (!get_gene(chromosome, start, end, gene_model, gene, msg)) {
          if (msg.empty()) {
            msg = Error::m(ERR_INVALID_GENE_LOCATION, chromosome, start, end, gene_model);
          }
          return false;
        }

        mongo::BSONObj gene_attributes = gene[KeyMapper::ATTRIBUTES()].Obj();
        if (!gene_attributes.hasElement(attribute_name)) {
          msg = Error::m(ERR_INVALID_GENE_ATTRIBUTE, attribute_name);
        }

        attibute_value = gene_attributes[attribute_name].String();

        return true;
      }

      bool get_gene(const std::string& chromosome, const Position start, const Position end, const std::string& gene_model,
                    mongo::BSONObj& gene, std::string& msg)
      {
        std::string norm_gene_model = utils::normalize_name(gene_model);

        const std::vector<std::string> genes_empty;
        const std::vector<std::string> gene_models = {norm_gene_model};
        const std::vector<std::string> chromosomes = {chromosome};
        mongo::Query query;

        if (!dba::genes::build_genes_database_query(chromosomes, start, end, genes_empty, gene_models, true, query, msg)) {
          msg = "";
          return false;
        }

        std::cerr << query.toString() << std::endl;
        return helpers::get_one(Collections::GENES(), query, gene);
      }

      bool get_genes(const std::vector<std::string> &chromosomes, const Position start, const Position end, const std::vector<std::string>& genes_names_or_id,
                     const std::string &user_key, const std::vector<std::string> &norm_gene_models,  std::vector<mongo::BSONObj>& genes, std::string &msg)
      {
        mongo::Query query;
        if (!dba::genes::build_genes_database_query(chromosomes, start, end, genes_names_or_id, norm_gene_models, false, query, msg)) {
          return false;
        }

        std::cerr << query.toString() << std::endl;

        std::vector<mongo::BSONObj> genes_db_objs;
        if (!helpers::get(Collections::GENES(), query, genes_db_objs, msg)) {
          return false;
        }

        for (const auto& gene_db_obj : genes_db_objs) {
          mongo::BSONObjBuilder gene_builder;
          gene_builder.append("chromosome", gene_db_obj[KeyMapper::CHROMOSOME()].String());
          gene_builder.append("start", gene_db_obj[KeyMapper::START()].numberLong());
          gene_builder.append("end", gene_db_obj[KeyMapper::END()].numberLong());
          gene_builder.append("source", gene_db_obj[KeyMapper::SOURCE()].String());
          gene_builder.append("feature", gene_db_obj[KeyMapper::FEATURE()].String());
          gene_builder.append("score", gene_db_obj[KeyMapper::SCORE()].numberDouble());
          gene_builder.append("strand", gene_db_obj[KeyMapper::STRAND()].String());
          gene_builder.append("frame", gene_db_obj[KeyMapper::FRAME()].String());

          const mongo::BSONObj& attributes = gene_db_obj[KeyMapper::ATTRIBUTES()].Obj();
          gene_builder.appendElements(attributes);

          genes.emplace_back(gene_builder.obj());
        }

        return true;
      }

      bool get_genes_from_database(const std::vector<std::string> &chromosomes, const Position start, const Position end,
                                   const std::vector<std::string>& genes, const std::string& norm_gene_model,
                                   ChromosomeRegionsList& chromosomeRegionsList, std::string& msg )
      {

        std::vector<std::string> gene_models;
        gene_models.push_back(norm_gene_model);

        mongo::Query query;

        if (!build_genes_database_query(chromosomes, start, end, genes, gene_models, false, query, msg)) {
          return false;
        }

        std::string collection = dba::helpers::collection_name(dba::Collections::GENES());

        Connection c;
        auto data_cursor = c->query(collection, query);

        std::string actual_chromosome("");
        Regions actual_regions;

        while (data_cursor->more()) {
          mongo::BSONObj gene = data_cursor->next().getOwned();
          mongo::BSONObj::iterator e_it = gene.begin();

          std::string gene_id = e_it.next().String();
          DatasetId dataset_id = e_it.next().numberInt();
          std::string chromosome = e_it.next().String();
          std::string source = e_it.next().String();
          std::string feature = e_it.next().String();
          Position start = e_it.next().numberInt();
          Position end = e_it.next().numberInt();
          Score score = e_it.next().numberDouble();
          std::string strand = e_it.next().String();
          std::string frame = e_it.next().String();
          datatypes::Metadata attributes = datatypes::bson_to_metadata(gene[KeyMapper::ATTRIBUTES()].Obj());

          if (chromosome != actual_chromosome) {
            if (!actual_chromosome.empty() && !actual_regions.empty()) {
              chromosomeRegionsList.emplace_back(std::move(actual_chromosome), std::move(actual_regions));
            }
            actual_chromosome = chromosome;
            actual_regions = Regions();
          }

          actual_regions.emplace_back(build_gene_region(start, end, dataset_id, source, score, feature,  strand, frame, attributes));
        }

        if (!actual_chromosome.empty()) {
          chromosomeRegionsList.emplace_back(std::move(actual_chromosome), std::move(actual_regions));
        }

        c.done();
        return true;
      }

      bool get_gene_expressions_from_database(const std::vector<std::string> &sample_ids, const  std::vector<long>& replicas,
                                              const std::vector<std::string> &genes,
                                              const std::string& norm_gene_model,  ChromosomeRegionsList& chromosomeRegionsList, std::string& msg)
      {
        Connection c;
        mongo::BSONObj gene_model_obj = c->findOne(dba::helpers::collection_name(dba::Collections::GENE_MODELS()),
                                        BSON("norm_name" << norm_gene_model));

        if (gene_model_obj.isEmpty()) {
          msg = "gene model " + norm_gene_model + " does not exists";
          c.done();
          return false;
        }

        mongo::BSONArray ges_datasets = helpers::build_dataset_ids_arrays(Collections::GENE_EXPRESSIONS(), BSON(
                                          "sample_id" << BSON("$in" << utils::build_array(sample_ids))  <<
                                          "replica" << BSON("$in" << utils::build_array_long(replicas))
                                        ));

        mongo::BSONObjBuilder bob;
        bob.append(KeyMapper::DATASET(), BSON("$in" << ges_datasets));
        if (!genes.empty()) {
          bob.append(KeyMapper::TRACKING_ID(), BSON("$in" << utils::build_array(genes)));
        }

        auto query = bob.obj();
        std::cerr << query.toString() << std::endl;

        std::string collection = dba::helpers::collection_name(dba::Collections::GENE_SINGLE_EXPRESSIONS());
        auto data_cursor = c->query(collection, query);
        while (data_cursor->more()) {
          mongo::BSONObj gene = data_cursor->next().getOwned();

          mongo::BSONObj::iterator e_it = gene.begin();

          std::string gene_expression_id = e_it.next().String();
          DatasetId dataset_id = e_it.next().numberInt();
          std::string tracking_id = e_it.next().String();
          std::string gene_id = e_it.next().String();
          std::string gene_short_name = e_it.next().String();
          Score fpkm = e_it.next().numberDouble();
          Score fpkm_lo = e_it.next().numberDouble();
          Score fpkm_hi = e_it.next().numberDouble();
          std::string fpkm_status = e_it.next().String();

          std::cerr << gene_expression_id << dataset_id << tracking_id << gene_id << gene_short_name << fpkm << fpkm_lo << fpkm_hi << fpkm_status << std::endl;
        }

        c.done();

        return true;
      }

      bool map_gene_location(const std::string& gene_id, const std::string& gene_model,
                             std::string& chromosome, Position& start, Position& end, std::string& msg)
      {
        chromosome = "chr1";
        start = 1;
        end = 2;
        return true;
      }
    }
  }
}
