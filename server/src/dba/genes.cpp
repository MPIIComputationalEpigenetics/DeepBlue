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

#include <boost/algorithm/string.hpp>

#include "../connection/connection.hpp"

#include "../datatypes/metadata.hpp"

#include "../extras/utils.hpp"

#include "../parser/gtf.hpp"

#include "../interfaces/serializable.hpp"

#include "annotations.hpp"
#include "collections.hpp"
#include "data.hpp"
#include "full_text.hpp"
#include "genes.hpp"
#include "helpers.hpp"
#include "info.hpp"
#include "remove.hpp"
#include "users.hpp"

#include "../errors.hpp"

namespace epidb {
  namespace dba {
    namespace genes {

      mongo::BSONObj build_gene_info(mongo::BSONObj gene_db_obj)
      {
        mongo::BSONObjBuilder gene_builder;

        gene_builder.append(gene_db_obj["_id"]);
        gene_builder.append("chromosome", gene_db_obj[KeyMapper::CHROMOSOME()].String());
        gene_builder.append("start", gene_db_obj[KeyMapper::START()].numberLong());
        gene_builder.append("end", gene_db_obj[KeyMapper::END()].numberLong());
        gene_builder.append("source", gene_db_obj[KeyMapper::SOURCE()].String());
        gene_builder.append("score", gene_db_obj[KeyMapper::SCORE()].numberDouble());
        gene_builder.append("strand", gene_db_obj[KeyMapper::STRAND()].String());
        gene_builder.append("frame", gene_db_obj[KeyMapper::FRAME()].String());
        const mongo::BSONObj& attributes = gene_db_obj[KeyMapper::ATTRIBUTES()].Obj();
        gene_builder.appendElements(attributes);

        return gene_builder.obj();
      }

      bool gene_model_info(const std::string& id, mongo::BSONObj& obj_metadata, std::string& msg)
      {
        mongo::BSONObj data_obj;
        if (!data::gene_model(id, data_obj, msg)) {
          return false;
        }

        mongo::BSONObjBuilder bob;

        bob.append(data_obj["_id"]);
        bob.append(data_obj["description"]);
        bob.append(data_obj["format"]);
        bob.append(data_obj["upload_info"]["total_genes"]);
        bob.append(data_obj["extra_metadata"]);

        obj_metadata = bob.obj();

        return true;
      }

      bool gene_info(const std::string& id, mongo::BSONObj& obj_metadata, std::string& msg)
      {
        mongo::BSONObj data_obj;
        if (!data::gene(id, data_obj, msg)) {
          return false;
        }

        obj_metadata = build_gene_info(data_obj);

        return true;
      }

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

      mongo::BSONObj to_bson(const int dataset_id, const std::string& gene_id, const parser::GTFRow& row)
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

        std::vector<mongo::BSONObj> rows_obj_bulk;

        for (const auto& row :  gtf->rows()) {

          int _id;
          if (!helpers::get_increment_counter("genes", _id, msg) ||
              !helpers::notify_change_occurred(Collections::GENES(), msg))  {
            return false;
          }

          std::string gene_id = "gn" + utils::integer_to_string(_id);
          mongo::BSONObj row_obj = to_bson(dataset_id, gene_id, row);

          rows_obj_bulk.emplace_back(std::move(row_obj));
          total_genes++;
        }

        mongo::BSONObjBuilder index_name;
        index_name.append(KeyMapper::CHROMOSOME(), 1);
        index_name.append(KeyMapper::START(), 1);
        index_name.append(KeyMapper::DATASET(), 1);
        c->createIndex(helpers::collection_name(Collections::GENES()), index_name.obj());
        if (!c->getLastError().empty()) {
          msg = c->getLastError();
          c.done();
          return false;
        }

        c->insert(helpers::collection_name(Collections::GENES()), rows_obj_bulk);
        if (!c->getLastError().empty()) {
          msg = c->getLastError();
          c.done();
          return false;
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

      bool build_genes_database_query(const std::vector<std::string> &chromosomes, const int start, const int end, const std::string& strand,
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
          mongo::BSONArray genes_array = utils::build_array(genes);
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

        if (!strand.empty()) {
          bob.append(KeyMapper::STRAND(), strand);
        }

        mongo::BSONObj filter = bob.obj();

        query = mongo::Query(filter).sort(BSON(KeyMapper::CHROMOSOME() << 1 << KeyMapper::START() << 1));

        return true;
      }

      bool get_gene_attribute(const std::string& chromosome, const Position start, const Position end, const std::string& strand,
                              const std::string& attribute_name, const std::string& gene_model,
                              std::string& attibute_value, std::string& msg)
      {
        std::string norm_gene_model = utils::normalize_name(gene_model);
        RegionPtr gene;
        if (!get_gene_by_location(chromosome, start, end, strand, norm_gene_model, gene, msg)) {
          return false;
        }

        auto it = gene->attributes().find(attribute_name);
        if (it == gene->attributes().end()) {
          msg = Error::m(ERR_INVALID_GENE_ATTRIBUTE, attribute_name);
          return false;
        }

        attibute_value = it->second;

        return true;
      }

      std::unordered_map<int, std::string> id_to_gene_model;
      bool get_gene_model_by_dataset_id(const int dataset_id, std::string& name, std::string& msg)
      {
        auto it = id_to_gene_model.find(dataset_id);
        if (it == id_to_gene_model.end()) {
          mongo::BSONObj gene_model_obj;
          if (!helpers::get_one(Collections::GENE_MODELS(), mongo::Query(BSON(KeyMapper::DATASET() << dataset_id)), gene_model_obj)) {
            msg = Error::m(ERR_INVALID_GENE_MODEL_ID, dataset_id);
            return false;
          }
          name = gene_model_obj["name"].String();
          id_to_gene_model[dataset_id] = name;
        } else {
          name = it->second;
        }
        return true;
      }


      bool get_genes(const std::vector<std::string> &chromosomes, const Position start, const Position end, const std::string& strand, const std::vector<std::string>& genes_names_or_id,
                     const std::string &user_key, const std::vector<std::string> &norm_gene_models,  std::vector<mongo::BSONObj>& genes, std::string &msg)
      {
        mongo::Query query;
        if (!dba::genes::build_genes_database_query(chromosomes, start, end, strand, genes_names_or_id, norm_gene_models, false, query, msg)) {
          return false;
        }

        std::vector<mongo::BSONObj> genes_db_objs;
        if (!helpers::get(Collections::GENES(), query, genes_db_objs, msg)) {
          return false;
        }

        for (const auto& gene_db_obj : genes_db_objs) {
          genes.emplace_back(build_gene_info(gene_db_obj));
        }

        return true;
      }

      bool get_genes_from_database(const std::vector<std::string> &chromosomes, const Position start, const Position end, const std::string& strand,
                                   const std::vector<std::string>& genes, const std::string& norm_gene_model,
                                   ChromosomeRegionsList& chromosomeRegionsList, std::string& msg )
      {
        std::vector<std::string> gene_models;
        gene_models.push_back(norm_gene_model);

        mongo::Query query;

        if (!build_genes_database_query(chromosomes, start, end, strand, genes, gene_models, false, query, msg)) {
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

      struct GeneLocation {
        std::string chromosome;
        Position start;
        Position end;
        std::string strand;

        GeneLocation(std::string c, Position s, Position e, std::string d) :
          chromosome(c),
          start(s),
          end(e),
          strand(d) {}

        bool operator==(const GeneLocation &other) const
        {
          return (chromosome == other.chromosome
                  && start == other.start
                  && end == other.end
                  && strand == other.strand);
        }
      };

      struct GeneLocationHasher {
        std::size_t operator()(const GeneLocation& k) const
        {
          using std::hash;

          size_t res = 17;
          res = res * 31 + hash<std::string>()( k.chromosome );
          res = res * 31 + hash<Position>()( k.start );
          res = res * 31 + hash<Position>()( k.end );
          res = res * 31 + hash<std::string>()( k.strand );
          return res;
        }
      };

      typedef std::unordered_map<std::string, GeneLocation> GeneModelCache;
      typedef std::unordered_map<std::string, GeneModelCache> GeneModelsCache;

      typedef std::unordered_map<GeneLocation, RegionPtr, GeneLocationHasher> GeneLocationCache;
      typedef std::unordered_map<std::string, GeneLocationCache> GeneLocationsCache;

      bool load_gene_model(const std::string & norm_gene_model,
                           GeneModelCache & cache_tracking_id, GeneModelCache & cache_gene_name,
                           GeneLocationCache & cache_location_gene, std::string & msg)
      {
        std::vector<std::string> chromosomes;
        std::vector<std::string> genes;
        ChromosomeRegionsList chromosomeRegionsList;

        if (!get_genes_from_database(chromosomes, -1, -1, "", genes, norm_gene_model, chromosomeRegionsList, msg)) {
          return false;
        }

        for (const auto &chromosomes_regions : chromosomeRegionsList) {
          const std::string &chromosome = chromosomes_regions.first;
          const Regions &regions = chromosomes_regions.second;
          for (const auto& r : regions) {
            // We have to clone because the original gene_region is an unique_ptr and it will be destroyed.
            auto region = r->clone();

            const GeneRegion* gene_region = static_cast<const GeneRegion *>(region.get());
            const std::string& gene_id = gene_region->attributes().at("gene_id");
            const std::string& gene_name = gene_region->attributes().at("gene_name");

            std::vector<std::string> gene_id_parts;
            boost::split(gene_id_parts, gene_id, boost::is_any_of("."));
            /* Not supported by GCC 4.7 (MPI's compiler)
            cache.emplace(std::piecewise_construct,
                          std::forward_as_tuple(gene_id),
                          std::forward_as_tuple(chromosome, gene_region->start(), gene_region->end()));
            */
            // See the get_string in gene_regions
            auto gl = GeneLocation(chromosome, gene_region->start(), gene_region->end(), gene_region->get_string(3));
            cache_tracking_id.insert( std::make_pair(gene_id_parts[0], gl));
            cache_gene_name.insert( std::make_pair(gene_name, gl));

            cache_location_gene.insert( std::make_pair(gl,  std::move(region)));
          }
        }

        return true;
      }

      GeneModelsCache gene_models_cache_tracking_id;
      GeneModelsCache gene_models_cache_gene_name;
      GeneLocationsCache gene_locations_cache;

      void invalidate_cache() {
        gene_models_cache_tracking_id.clear();
        gene_models_cache_gene_name.clear();
        gene_locations_cache.clear();
      }

      bool load_gene_location_caches(const std::string & gene_model, std::string &msg)
      {
        GeneModelCache cache_tracking_id;
        GeneModelCache cache_gene_name;
        GeneLocationCache gene_location_cache;
        if (!load_gene_model(gene_model, cache_tracking_id, cache_gene_name, gene_location_cache, msg)) {
          return false;
        }
        gene_models_cache_tracking_id.insert(std::make_pair(gene_model, cache_tracking_id));
        gene_models_cache_gene_name.insert(std::make_pair(gene_model, cache_gene_name));
        gene_locations_cache.insert(std::make_pair(gene_model, std::move(gene_location_cache)));

        return true;
      }

      bool get_gene_by_location(const std::string& chromosome, const Position start, const Position end, const std::string& strand,
                                const std::string& gene_model, RegionPtr& gene, std::string& msg)
      {
        // Put a lock here
        if (gene_locations_cache.find(gene_model) == gene_locations_cache.end()) {
          if (!load_gene_location_caches(gene_model, msg)) {
            return false;
          }
        }

        const auto &cache = gene_locations_cache[gene_model];
        auto it = cache.find({chromosome, start, end, strand});

        if (it == cache.end()) {
          msg = Error::m(ERR_INVALID_GENE_LOCATION, chromosome, start, end, gene_model);
          return false;
        }

        gene = it->second->clone();

        return true;
      }


      bool map_gene_location(const std::string & gene_tracking_id,
                             const std::string & gene_name, const std::string & gene_model,
                             std::string & chromosome, Position & start, Position & end, std::string& strand, std::string & msg)
      {
        // Put a lock here
        if (gene_models_cache_tracking_id.find(gene_model) == gene_models_cache_tracking_id.end()) {
          if (!load_gene_location_caches(gene_model, msg)) {
            return false;
          }
        }
        //

        std::vector<std::string> gene_tracking_id_parts;
        boost::split(gene_tracking_id_parts, gene_tracking_id, boost::is_any_of("."));
        const auto &cache_tracking_id = gene_models_cache_tracking_id[gene_model];
        auto it = cache_tracking_id.find(gene_tracking_id_parts[0]);

        if (it == cache_tracking_id.end()) {
          const auto &cache_gene_name = gene_models_cache_gene_name[gene_model];
          it = cache_gene_name.find(gene_name);
          if (it == cache_gene_name.end()) {
            if (gene_tracking_id.empty()) {
              msg = "Gene name '" + gene_name + "' not found in the gene model " + gene_model;
            } else {
              msg = "Gene tracking_id '" + gene_tracking_id + "' with gene name: '" + gene_name + "' not found in the gene model " + gene_model;
            }
            return false;
          }
        }

        const auto &region = it->second;
        chromosome = region.chromosome;
        start = region.start;
        end = region.end;
        strand = region.strand;

        return true;
      }
    }
  }
}
