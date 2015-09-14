//
//  genes.cpp
//  epidb
//
//  Created by Felipe Albrecht on 09.09.2015
//  Copyright (c) 2015 Max Planck Institute for Computer Science. All rights reserved.
//


#include <string>

#include <mongo/bson/bson.h>

#include "../connection/connection.hpp"

#include "../datatypes/metadata.hpp"

#include "../parser/gtf.hpp"

#include "../extras/utils.hpp"

#include "annotations.hpp"
#include "collections.hpp"
#include "full_text.hpp"
#include "helpers.hpp"
#include "remove.hpp"
#include "users.hpp"

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
                          std::string &gene_set_id,
                          mongo::BSONObj &gene_set_metadata,
                          std::string &msg)
      {
        if (!helpers::get_increment_counter("datasets", dataset_id, msg) ||
            !helpers::notify_change_occurred("datasets", msg))  {
          return false;
        }

        int _id;
        if (!helpers::get_increment_counter(Collections::GENE_SETS(), _id, msg) ||
            !helpers::notify_change_occurred(Collections::GENE_SETS(), msg))  {
          return false;
        }
        gene_set_id = "gs" + utils::integer_to_string(_id);

        mongo::BSONObjBuilder gene_set_metadata_builder;
        gene_set_metadata_builder.append("_id", gene_set_id);
        gene_set_metadata_builder.append(KeyMapper::DATASET(), dataset_id);
        gene_set_metadata_builder.append("name", name);
        gene_set_metadata_builder.append("norm_name", norm_name);
        gene_set_metadata_builder.append("description", description);
        gene_set_metadata_builder.append("norm_description", norm_description);
        gene_set_metadata_builder.append("extra_metadata", extra_metadata_obj);

        gene_set_metadata = gene_set_metadata_builder.obj();
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

      mongo::BSONObj to_bson(const int dataset_id, const std::string& gene_set_id, const std::string& gene_id, const parser::GTFRow& row)
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
                  std::string &gene_set_id, std::string &msg)
      {
        mongo::BSONObj gene_set_metadata;
        mongo::BSONObj extra_metadata_obj = datatypes::metadata_to_bson(extra_metadata);
        int dataset_id;

        if (!build_metadata(name, norm_name, description, norm_description, "GTF", extra_metadata_obj,
                            user_key, ip, dataset_id, gene_set_id, gene_set_metadata, msg)) {
          return false;
        }

        mongo::BSONObj upload_info;
        if (!build_upload_info(user_key, ip, "GTF", upload_info, msg)) {
          return false;
        }
        mongo::BSONObjBuilder gene_set_builder;
        gene_set_builder.appendElements(gene_set_metadata);
        gene_set_builder.append("upload_info", upload_info);

        mongo::BSONObj e = gene_set_builder.obj();
        Connection c;
        c->insert(helpers::collection_name(Collections::GENE_SETS()), e);
        if (!c->getLastError().empty()) {
          msg = c->getLastError();
          c.done();
          return false;
        }

        if (!search::insert_full_text(Collections::GENE_SETS(), gene_set_id, gene_set_metadata, msg)) {
          c.done();
          std::string new_msg;
          if (!remove::gene_set(gene_set_id, user_key, new_msg)) {
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
          mongo::BSONObj row_obj = to_bson(dataset_id, gene_set_id, gene_id, row);

          c->insert(helpers::collection_name(Collections::GENES()), row_obj);
          if (!c->getLastError().empty()) {
            msg = c->getLastError();
            c.done();
            return false;
          }

          total_genes++;
        }

        if (!update_upload_info(Collections::GENE_SETS(), gene_set_id, total_size, total_genes, msg)) {
          std::string new_msg;
          if (!remove::gene_set(gene_set_id, user_key, new_msg)) {
            msg = msg + " " + new_msg;
          }
          return false;
        }

        c.done();
        return true;
      }

      bool get_genes_from_database(const std::vector<std::string>& genes, const std::string& gene_set,
                                   ChromosomeRegionsList& chromosomeRegionsList, std::string& msg )
      {
        Connection c;

        mongo::BSONArray genes_array = dba::helpers::build_array(genes);
        mongo::BSONArray genes_array_2 = genes_array;

        mongo::BSONObj gene_set_obj = c->findOne(dba::helpers::collection_name(dba::Collections::GENE_SETS()), BSON("norm_name" << gene_set));

        if (gene_set_obj.isEmpty()) {
          msg = "gene set " + gene_set + " does not exists";
          c.done();
          return false;
        }

        auto dataset_id = gene_set_obj[KeyMapper::DATASET()].Int();

        mongo::BSONObj b_in_gene_name = BSON((KeyMapper::ATTRIBUTES() + ".gene_name") << BSON("$in" << genes_array));
        mongo::BSONObj b_in_gene_id = BSON((KeyMapper::ATTRIBUTES() + ".gene_id") << BSON("$in" << genes_array_2));

        mongo::BSONObjBuilder bob;
        bob.append(KeyMapper::DATASET(), dataset_id);
        bob.append("$or", BSON_ARRAY(b_in_gene_name << b_in_gene_id));
        mongo::BSONObj filter = bob.obj();

        mongo::Query query = mongo::Query(filter).sort(BSON(KeyMapper::CHROMOSOME() << 1 << KeyMapper::START() << 1));
        std::string collection = dba::helpers::collection_name(dba::Collections::GENES());
        std::auto_ptr<mongo::DBClientCursor> data_cursor = c->query(collection, query);

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
          Position score = e_it.next().numberDouble();
          char strand = e_it.next().String()[0];
          char frame = e_it.next().String()[0];
          datatypes::Metadata attributes = datatypes::bson_to_metadata(gene[KeyMapper::ATTRIBUTES()].Obj());

          if (chromosome != actual_chromosome) {
            if (!chromosome.empty() && !actual_regions.empty()) {
              chromosomeRegionsList.emplace_back(std::move(chromosome), std::move(actual_regions));
            }
            actual_chromosome = chromosome;
            actual_regions = build_regions();
          }

          RegionPtr region = build_gene_region(start, end, dataset_id, source, score, strand, frame, attributes);
          actual_regions.push_back(std::move(region));
        }

        if (!actual_chromosome.empty()) {
          chromosomeRegionsList.emplace_back(std::move(actual_chromosome), std::move(actual_regions));
        }

        c.done();
        return true;
      }

    }
  }
}