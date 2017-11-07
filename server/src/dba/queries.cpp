//
//  queries.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 09.07.13.
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

#include <ctime>
#include <cstring>
#include <set>
#include <iterator>
#include <unordered_map>
#include <utility>

#include <boost/thread.hpp>

#include <mongo/bson/bson.h>

#include "../algorithms/algorithms.hpp"
#include "../algorithms/filter.hpp"

#include "../cache/column_dataset_cache.hpp"
#include "../cache/queries_cache.hpp"

#include "../connection/connection.hpp"

#include "../datatypes/column_types_def.hpp"
#include "../datatypes/expressions_manager.hpp"
#include "../datatypes/regions.hpp"

#include "../dba/experiments.hpp"

#include "../extras/utils.hpp"

#include "annotations.hpp"
#include "collections.hpp"
#include "dba.hpp"
#include "genes.hpp"
#include "genomes.hpp"
#include "helpers.hpp"
#include "list.hpp"
#include "key_mapper.hpp"
#include "metafield.hpp"
#include "retrieve.hpp"
#include "users.hpp"

#include "queries.hpp"

#include "../errors.hpp"
#include "../log.hpp"
#include "../macros.hpp"

namespace epidb {
  namespace dba {
    namespace query {

      // TODO: Merge wtth users cache and use templates
      class NameDatasetIdCache {
      private:
        boost::shared_mutex _access;
        std::unordered_map<std::string, DatasetId> name_dataset_id;
        std::unordered_map<DatasetId, std::string> dataset_id_name;

      public:

        DatasetId get_dataset_id(const std::string &name)
        {
          boost::shared_lock< boost::shared_mutex > lock(_access);

          return name_dataset_id[name];
        }

        const std::string& get_name(const DatasetId id)
        {
          boost::shared_lock< boost::shared_mutex > lock(_access);

          return dataset_id_name[id];
        }

        void set(const std::string &name, const DatasetId id)
        {
          boost::unique_lock< boost::shared_mutex > lock(_access);

          name_dataset_id[name] = id;
          dataset_id_name[id] = name;
        }

        bool exists_dataset_id(const std::string &name)
        {
          boost::shared_lock< boost::shared_mutex > lock(_access);

          if (name_dataset_id.find(name) != name_dataset_id.end()) {
            return true;
          }
          return false;
        }

        bool exists_name(const DatasetId id)
        {
          boost::shared_lock< boost::shared_mutex > lock(_access);

          if (dataset_id_name.find(id) != dataset_id_name.end()) {
            return true;
          }
          return false;
        }

        void invalidate()
        {
          boost::unique_lock< boost::shared_mutex > lock(_access);
          name_dataset_id.clear();
        }
      };

      NameDatasetIdCache experiment_name_dataset_id_cache;
      NameDatasetIdCache annotation_pattern_cache;

      void invalidate_cache()
      {
        experiment_name_dataset_id_cache.invalidate();
        annotation_pattern_cache.invalidate();
      }

      mongo::BSONObj reduce_args(const mongo::BSONObj &args)
      {
        // Keys to be removed
        std::set<std::string> keys = {"annotation",
                                      "genome", "genomes",
                                      "experiment_name", "epigenetic_mark",
                                      "project", "technique"
                                     };
        mongo::BSONObjBuilder bob;

        for (auto it = args.begin(); it.more(); ) {
          mongo::BSONElement e = it.next();
          std::string field_name = std::string(e.fieldName());
          if (keys.find(field_name) == keys.end()) {
            bob.append(e);
          }
        }

        return bob.obj();
      }


      bool store_query(const datatypes::User& user,
                       const std::string &type, const mongo::BSONObj &args,
                       std::string &query_id, std::string &msg)
      {
        mongo::BSONObjBuilder search_query_builder;

        mongo::BSONObj query_args = reduce_args(args);

        search_query_builder.append("type", type);
        search_query_builder.append("user", user.id());
        search_query_builder.append("query_args", query_args);

        if (query_args.hasField("norm_experiment_name")) {
          search_query_builder.append("query_args.norm_experiment_name",
                                      utils::build_array(
                                        utils::build_vector(
                                          query_args["norm_experiment_name"].Array()
                                        )
                                      ));
        }

        auto search_query = search_query_builder.obj();
        mongo::BSONObj result;
        if (helpers::get_one(Collections::QUERIES(), search_query, result)) {
          query_id = result["_id"].String();
          return true;
        }

        int query_counter;
        if (!helpers::get_increment_counter(Collections::QUERIES(), query_counter, msg)) {
          return false;
        }
        query_id = "q" + utils::integer_to_string(query_counter);
        time_t time_;
        time(&time_);

        mongo::BSONObjBuilder stored_query_builder;

        stored_query_builder.append("_id", query_id);
        stored_query_builder.append("type", type);
        stored_query_builder.append("user", user.id());
        stored_query_builder.appendTimeT("time", time_);
        stored_query_builder.append("args", args);
        stored_query_builder.append("query_args", query_args);

        Connection c;

        c->insert(helpers::collection_name(Collections::QUERIES()), stored_query_builder.obj());
        if (!c->getLastError().empty()) {
          msg = c->getLastError();
          c.done();
          return false;
        }
        c.done();
        return true;
      }

      bool modify_query(const datatypes::User& user,
                        const std::string &query_id, const std::string &key, const std::string &value,
                        std::string &new_query_id, std::string &msg)
      {
        mongo::BSONObj old_query;
        if (!helpers::get_one(Collections::QUERIES(), BSON("_id" << query_id), old_query)) {
          msg = Error::m(ERR_INVALID_QUERY_ID, query_id);
          return false;
        }

        if (old_query["user"].String() != user.id()) {
          msg = "You do not have permission to change this query.";
          return false;
        }

        mongo::BSONObj old_args = old_query["args"].Obj();
        mongo::BSONObjBuilder new_args_builder;

        new_args_builder.append(key, value);
        new_args_builder.appendElementsUnique(old_args);

        mongo::BSONObj new_args = new_args_builder.obj();

        // Check if the modified query document already exists
        mongo::BSONObjBuilder search_query_builder;
        search_query_builder.append("type", old_query["type"].String());
        search_query_builder.append("user", user.id());
        search_query_builder.append("derived_from", query_id);
        search_query_builder.append("args", new_args);

        mongo::BSONObj search_obj = search_query_builder.obj();

        Connection c;
        mongo::BSONObj query_obj = c->findOne(dba::helpers::collection_name(dba::Collections::QUERIES()), search_obj);

        if (!query_obj.isEmpty()) {
          c.done();
          new_query_id = query_obj["_id"].String();
          return true;
        }

        // Create a new query document
        int query_counter;
        if (!helpers::get_increment_counter(Collections::QUERIES(), query_counter, msg)) {
          c.done();
          return false;
        }
        new_query_id = "q" + utils::integer_to_string(query_counter);
        time_t time_;
        time(&time_);

        mongo::BSONObjBuilder stored_query_builder;
        stored_query_builder.append("_id", new_query_id);
        stored_query_builder.append("user", user.id());
        stored_query_builder.appendTimeT("time", time_);
        stored_query_builder.append("type", old_query["type"].String());
        stored_query_builder.append("derived_from", query_id);
        stored_query_builder.append("args", new_args);

        c->insert(helpers::collection_name(Collections::QUERIES()), stored_query_builder.obj());
        if (!c->getLastError().empty()) {
          msg = c->getLastError();
          c.done();
          return false;
        }
        c.done();
        return true;
      }

      bool retrieve_query(const datatypes::User& user,
                          const std::string &query_id,
                          processing::StatusPtr status, ChromosomeRegionsList &regions, std::string &msg,
                          bool reduced_mode)
      {
        processing::RunningOp runningOp = status->start_operation(processing::PROCESS_QUERY);
        if (is_canceled(status, msg)) {
          return false;
        }

        mongo::BSONObj query;
        if (!helpers::get_one(Collections::QUERIES(), BSON("_id" << query_id), query)) {
          msg = Error::m(ERR_INVALID_QUERY_ID, query_id);
          return false;
        }

        const mongo::BSONObj& args = query["args"].Obj();
        if (args.hasField("cache") && args["cache"].String() == "yes") {
          return cache::get_query_cache(user, query["derived_from"].String(), status, regions, msg);
        }

        const std::string& type = query["type"].str();

        if (type == "experiment_select") {
          if (!retrieve_experiment_select_query(user, query, status, regions, msg, reduced_mode)) {
            return false;
          }
        } else if (type == "intersect") {
          if (!retrieve_intersection_query(user, query, status, regions, msg)) {
            return false;
          }
        } else if (type == "overlap") {
          if (!retrieve_overlap_query(user, query, status, regions, msg)) {
            return false;
          }
        } else if (type == "flank") {
          if (!retrieve_flank_query(user, query, status, regions, msg)) {
            return false;
          }
        } else if (type == "extend") {
          if (!retrieve_extend_query(user, query, status, regions, msg)) {
            return false;
          }
        } else if (type == "merge") {
          if (!retrieve_merge_query(user, query, status, regions, msg)) {
            return false;
          }
        } else if (type == "annotation_select") {
          if (!retrieve_annotation_select_query(user, query, status, regions, msg)) {
            return false;
          }
        } else if (type == "genes_select") {
          if (!retrieve_genes_select_query(user, query, status, regions, msg)) {
            return false;
          }
        } else if (type == "find_motif") {
          if (!retrieve_find_motif_query(user, query, status, regions, msg)) {
            return false;
          }
        } else if (type == "expressions_select") {
          if (!retrieve_expression_select_query(user, query, status, regions, msg)) {
            return false;
          }
        } else if (type == "filter") {
          if (!retrieve_filter_query(user, query, status, regions, msg)) {
            return false;
          }
        } else if (type == "tiling") {
          if (!retrieve_tiling_query(query, status, regions, msg)) {
            return false;
          }
        } else if (type == "aggregate") {
          if (!process_aggregate(user, query, status, regions, msg)) {
            return false;
          }

        } else if (type == "input_regions") {
          if (!retrieve_query_region_set(query, status, regions, msg)) {
            return false;
          }

        } else {
          msg = Error::m(ERR_UNKNOW_QUERY_TYPE, type);
          return false;
        }

        return true;
      }

      bool get_experiments_by_query(const datatypes::User& user, const std::string &query_id,
                                    processing::StatusPtr status, std::vector<utils::IdName> &datasets, std::string &msg)
      {
        processing::RunningOp runningOp = status->start_operation(processing::GET_EXPERIMENT_BY_QUERY);
        if (is_canceled(status, msg)) {
          return false;
        }

        ChromosomeRegionsList chromossome_regions;
        if (!retrieve_query(user, query_id, status, chromossome_regions, msg)) {
          return false;
        }

        std::set<DatasetId> datasets_it_set;
        for (auto  &regions : chromossome_regions) {
          for (auto &region : regions.second) {
            datasets_it_set.insert(region->dataset_id());
          }
        }

        for (const DatasetId& dataset_id: datasets_it_set) {
          mongo::BSONObj obj;
          if (!cache::get_bson_by_dataset_id(dataset_id, obj, msg)) {
            return false;
          }

          std::string _id = obj["_id"].str();
          std::string name = obj["name"].str();
          utils::IdName p(_id, name);
          datasets.push_back(p);
        }

        return true;
      }


      bool count_regions(const datatypes::User& user,
                         const std::string &query_id,
                         processing::StatusPtr status, size_t &count, std::string &msg)
      {
        processing::RunningOp runningOp = status->start_operation(processing::COUNT_REGIONS);
        if (is_canceled(status, msg)) {
          return false;
        }

        std::vector<mongo::BSONObj> result;
        if (!helpers::get(Collections::QUERIES(), "_id", query_id, result, msg)) {
          return false;
        }
        if (result.empty()) {
          msg = "Query key is invalid";
          return false;
        }
        mongo::BSONObj query = result[0];
        mongo::BSONObj args = query["args"].Obj();

        ChromosomeRegionsList regions;
        if (!retrieve_query(user, query_id, status, regions, msg)) {
          return false;
        }

        count = count_regions(regions);

        return true;
      }

      bool build_experiment_query(const int start, const int end, const std::string &norm_experiment_name,
                                  mongo::BSONObj &regions_query, std::string &msg)
      {
        Connection c;

        DatasetId dataset_id;

        if (experiment_name_dataset_id_cache.exists_dataset_id(norm_experiment_name)) {
          dataset_id = experiment_name_dataset_id_cache.get_dataset_id(norm_experiment_name);
        } else {
          mongo::BSONObj experiment_obj;
          if (!dba::experiments::by_name(norm_experiment_name, experiment_obj, msg)) {
            return false;
          }
          mongo::BSONElement dataset_id_elem = experiment_obj[KeyMapper::DATASET()];
          dataset_id = dataset_id_elem.Int();
          experiment_name_dataset_id_cache.set(norm_experiment_name, dataset_id);
        }

        mongo::BSONObjBuilder regions_query_builder;
        regions_query_builder.append(KeyMapper::DATASET(), dataset_id);

        if (end > -1) {
          regions_query_builder.append(KeyMapper::START(), BSON("$lte" << end));
        }

        if (start > -1) {
          regions_query_builder.append(KeyMapper::END(), BSON("$gte" << start));
        }

        regions_query = regions_query_builder.obj();

        c.done();

        return true;
      }

      bool build_experiment_query(const int start, const int end, const mongo::BSONArray& datasets_array,
                                  mongo::BSONObj &regions_query, std::string &msg)
      {
        mongo::BSONObjBuilder regions_query_builder;

        regions_query_builder.append(KeyMapper::DATASET(), BSON("$in" << datasets_array ));
        regions_query_builder.append(KeyMapper::START(), BSON("$lte" << end));
        regions_query_builder.append(KeyMapper::END(), BSON("$gte" << start));
        regions_query = regions_query_builder.obj();
        return true;
      }

      bool build_experiment_query(const datatypes::User& user,
                                  const mongo::BSONObj &args,
                                  mongo::BSONObj &regions_query, std::string &msg)
      {
        mongo::BSONObj args_query;
        if (!list::build_list_experiments_bson_query(user, args, args_query, msg)) {
          return false;
        }

        mongo::BSONArray datasets_array = helpers::build_dataset_ids_arrays(Collections::EXPERIMENTS(), args_query);

        mongo::BSONObjBuilder regions_query_builder;
        regions_query_builder.append(KeyMapper::DATASET(), BSON("$in" << datasets_array));

        if (args.hasField("start") && args.hasField("end")) {
          regions_query_builder.append(KeyMapper::START(), BSON("$lte" << args["end"].Int()));
          regions_query_builder.append(KeyMapper::END(), BSON("$gte" << args["start"].Int()));
        } else if (args.hasField("start")) {
          regions_query_builder.append(KeyMapper::END(), BSON("$gte" << args["start"].Int()));
        } else if (args.hasField("end")) {
          regions_query_builder.append(KeyMapper::START(), BSON("$lte" << args["end"].Int()));
        }

        regions_query = regions_query_builder.obj();

        return true;
      }

      bool build_annotation_query(const mongo::BSONObj &query,
                                  mongo::BSONObj &regions_query,  std::string &msg)
      {
        mongo::BSONObj args = query["args"].Obj();

        mongo::BSONObjBuilder annotations_query_builder;
        mongo::BSONArrayBuilder datasets_array_builder;

        // TODO: do the same as build_experiment_query, not verifying the type()
        if (args["norm_annotation"].type() == mongo::Array) {
          annotations_query_builder.append("norm_name", BSON("$in" << args["norm_annotation"]));
        } else {
          annotations_query_builder.append("norm_name", args["norm_annotation"].str());
        }
        annotations_query_builder.append("upload_info.done", true);

        mongo::BSONObj annotation_query = annotations_query_builder.obj();
        mongo::BSONArray datasets_array = helpers::build_dataset_ids_arrays(Collections::ANNOTATIONS(), annotation_query);

        mongo::BSONObjBuilder regions_query_builder;
        regions_query_builder.append(KeyMapper::DATASET(), BSON("$in" << datasets_array));

        if (args.hasField("start") && args.hasField("end")) {
          regions_query_builder.append(KeyMapper::START(), BSON("$lte" << args["end"].Int()));
          regions_query_builder.append(KeyMapper::END(), BSON("$gte" << args["start"].Int()));
        } else if (args.hasField("start")) {
          regions_query_builder.append(KeyMapper::END(), BSON("$gte" << args["start"].Int()));
        } else if (args.hasField("end")) {
          regions_query_builder.append(KeyMapper::START(), BSON("$lte" << args["end"].Int()));
        }

        regions_query = regions_query_builder.obj();

        return true;
      }

      bool retrieve_experiment_select_query(const datatypes::User& user,
                                            const mongo::BSONObj &query,
                                            processing::StatusPtr status, ChromosomeRegionsList &regions, std::string &msg,
                                            bool reduced_mode)
      {
        processing::RunningOp runningOp = status->start_operation(processing::RETRIEVE_EXPERIMENT_SELECT_QUERY, query);
        if (is_canceled(status, msg)) {
          return false;
        }

        mongo::BSONObj regions_query;
        mongo::BSONObj args = query["args"].Obj();

        if (!build_experiment_query(user, args, regions_query, msg)) {
          return false;
        }

        std::set<std::string> genomes;
        if (args.hasField("norm_genomes")) {
          genomes = utils::build_set(args["norm_genomes"].Array());
        } else {
          std::vector<std::string> norm_names = utils::build_vector(args["norm_experiment_name"].Array());
          for (auto experiment_norm_name : norm_names) {
            std::string genome;
            if (!dba::experiments::get_genome(experiment_norm_name, genome, msg)) {
              return false;
            }
            genomes.insert(genome);
          }
        }

        std::vector<std::string> chromosomes;
        if (args.hasField("chromosomes")) {
          chromosomes = utils::build_vector(args["chromosomes"].Array());
        } else {
          std::set<std::string> chrom;
          if (!dba::genomes::get_chromosomes(genomes, chrom, msg)) {
            return false;
          }
          chromosomes = std::vector<std::string>(chrom.begin(), chrom.end());
        }

        std::vector<ChromosomeRegionsList> genome_regions;
        for (const auto& genome : genomes) {
          ChromosomeRegionsList reg;
          if (!retrieve::get_regions(genome, chromosomes, regions_query, false, status, reg, msg, reduced_mode)) {
            return false;
          }
          genome_regions.push_back(std::move(reg));
        }

        // merge region data of all genomes
        std::vector<ChromosomeRegionsList>::iterator rit = genome_regions.begin();
        if (rit == genome_regions.end()) {
          return true;
        }

        ChromosomeRegionsList &last = *rit;
        rit++;
        for (; rit != genome_regions.end(); ++rit) {
          last = algorithms::merge_chromosome_regions(last, *rit);
        }
        regions = std::move(last);

        return true;
      }


      bool retrieve_query_region_set(const mongo::BSONObj &query,
                                     processing::StatusPtr status, ChromosomeRegionsList &regions, std::string &msg)
      {
        mongo::BSONObj args = query["args"].Obj();
        processing::RunningOp runningOp = status->start_operation(processing::RETRIEVE_QUERY_REGION_SET, query);
        if (is_canceled(status, msg)) {
          return false;
        }

        mongo::BSONObj regions_query = BSON(KeyMapper::DATASET() << args["dataset_id"].Int());

        std::vector<std::string> chromosomes = utils::build_vector(args["chromosomes"].Array());
        std::string genome = args["norm_genome"].String();

        if (!retrieve::get_regions(genome, chromosomes, regions_query, false, status, regions, msg)) {
          return false;
        }

        return true;
      }

      bool retrieve_annotation_select_query(const datatypes::User& user,
                                            const mongo::BSONObj &query,
                                            processing::StatusPtr status, ChromosomeRegionsList &regions, std::string &msg)
      {
        processing::RunningOp runningOp = status->start_operation(processing::RETRIEVE_ANNOTATION_SELECT_QUERY, query);
        if (is_canceled(status, msg)) {
          return false;
        }

        mongo::BSONObj regions_query;
        if (!build_annotation_query(query, regions_query, msg)) {
          return false;
        }

        mongo::BSONObj args = query["args"].Obj();

        std::vector<mongo::BSONElement> genome_arr;
        if (args.hasField("norm_genomes")) {
          genome_arr = args["norm_genomes"].Array();
        } else if (args.hasField("norm_genome")) {
          genome_arr.push_back(args["norm_genome"]);
        }

        std::vector<ChromosomeRegionsList> genome_regions;
        std::vector<mongo::BSONElement>::iterator git;

        for (git = genome_arr.begin(); git != genome_arr.end(); ++git) {
          std::string genome = git->str();

          std::vector<std::string> chromosomes;
          if (args.hasField("chromosomes")) {
            chromosomes = utils::build_vector(args["chromosomes"].Array());
          } else {
            std::set<std::string> chrom;
            if (!dba::genomes::get_chromosomes(genome, chrom, msg)) {
              return false;
            }
            chromosomes = std::vector<std::string>(chrom.begin(), chrom.end());
          }

          ChromosomeRegionsList reg;
          if (!retrieve::get_regions(genome, chromosomes, regions_query, false, status, reg, msg)) {
            return false;
          }
          genome_regions.push_back(std::move(reg));
        }

        // merge region data of all genomes
        std::vector<ChromosomeRegionsList>::iterator rit = genome_regions.begin();
        ChromosomeRegionsList &last = *rit;
        rit++;
        for (; rit != genome_regions.end(); ++rit) {
          last = algorithms::merge_chromosome_regions(last, *rit);
        }
        regions = std::move(last);

        return true;
      }

      bool retrieve_genes_select_query(const datatypes::User& user,
                                       const mongo::BSONObj &query,
                                       processing::StatusPtr status, ChromosomeRegionsList &regions, std::string &msg)
      {
        processing::RunningOp runningOp = status->start_operation(processing::RETRIEVE_GENES_DATA, query);
        if (is_canceled(status, msg)) {
          return false;
        }

        mongo::BSONObj args = query["args"].Obj();

        std::vector<std::string> genes;
        if (args.hasElement("genes")) {
          genes = utils::build_vector(args["genes"].Array());
        }

        std::vector<std::string> go_terms;
        if (args.hasElement("go_terms")) {
          go_terms = utils::build_vector(args["go_terms"].Array());
        }

        // Honestly I dont like it, but since we changed these parameters and we already have a database with queries...
        // TODO: manually change the database.
        std::string gene_model;
        if (args.hasField("gene_set")) {
          gene_model = args["gene_set"].str();
        } else {
          gene_model = args["gene_model"].str();
        }

        std::vector<std::string> chromosomes;
        if (args.hasField("chromosomes")) {
          chromosomes = utils::build_vector(args["chromosomes"].Array());
        }

        int start;
        int end = std::numeric_limits<Position>::max();

        if (args.hasField("start")) {
          start = args["start"].Int();
        } else {
          start = std::numeric_limits<Position>::min();
        }

        if (args.hasField("end")) {
          end = args["end"].Int();
        } else {
          end = std::numeric_limits<Position>::max();
        }

        if (!genes::get_genes_from_database(chromosomes, start, end, "", genes, go_terms, gene_model, regions, msg)) {
          return false;
        }

        return true;
      }

      bool retrieve_expression_select_query(const datatypes::User& user,
                                            const mongo::BSONObj &query,
                                            processing::StatusPtr status, ChromosomeRegionsList &regions, std::string &msg)
      {
        processing::RunningOp runningOp = status->start_operation(processing::RETRIEVE_EXPRESSIONS_DATA, query);
        if (is_canceled(status, msg)) {
          return false;
        }

        mongo::BSONObj args = query["args"].Obj();

        const std::string& expression_type_name = args["expression_type"].str();

        GET_EXPRESSION_TYPE_MSG(expression_type_name, expression_type)

        std::vector<std::string> sample_ids;
        if (args.hasField("sample_ids")) {
          sample_ids = utils::build_vector(args["sample_ids"].Array());
        }

        std::vector<long> replicas;
        if (args.hasField("replicas")) {
          replicas = utils::build_vector_long(args["replicas"].Array());
        }

        std::vector<std::string> genes;
        if (args.hasField("genes")) {
          genes = utils::build_vector(args["genes"].Array());
        }

        std::vector<std::string> project;
        if (args.hasField("norm_project")) {
          project = utils::build_vector(args["norm_project"].Array());
        }

        // Honestly I dont like it, but since we changed these parameters and we already have a database with queries...
        // TODO: manually change the database.
        std::string gene_model;
        if (args.hasField("gene_set")) {
          gene_model = args["gene_set"].str();
        } else {
          gene_model = args["gene_model"].str();
        }

        if (!expression_type->load_data(sample_ids, replicas, genes, project, gene_model, regions, msg)) {
          return false;
        }

        return true;
      }

      bool retrieve_find_motif_query(const datatypes::User& user,
                                     const mongo::BSONObj &query,
                                     processing::StatusPtr status, ChromosomeRegionsList &regions, std::string &msg)
      {
        processing::RunningOp runningOp = status->start_operation(processing::RETRIEVE_FIND_MOTIF_QUERY, query);
        if (is_canceled(status, msg)) {
          return false;
        }

        mongo::BSONObj regions_query;
        mongo::BSONObj args = query["args"].Obj();

        std::string motif = args["motif"].String();
        std::string genome = args["norm_genome"].String();
        bool overlap = args["overlap"].Bool();
        long long start = args["start"].Number();
        long long end = args["end"].Number();

        std::vector<std::string> chromosomes;
        if (args.hasField("chromosomes")) {
          chromosomes = utils::build_vector(args["chromosomes"].Array());
        } else {
          std::set<std::string> chrom;
          if (!dba::genomes::get_chromosomes(genome, chrom, msg)) {
            return false;
          }
          chromosomes = std::vector<std::string>(chrom.begin(), chrom.end());
        }

        if (!process_pattern(genome, motif, overlap, chromosomes, start, end, regions, msg)) {
          return false;
        }

        return true;
      }

      bool retrieve_intersection_query(const datatypes::User& user,
                                       const mongo::BSONObj &query,
                                       processing::StatusPtr status, ChromosomeRegionsList &regions, std::string &msg)
      {
        processing::RunningOp runningOp = status->start_operation(processing::RETRIEVE_INTERSECTION_QUERY, query);
        if (is_canceled(status, msg)) {
          return false;
        }

        mongo::BSONObj args = query["args"].Obj();

        mongo::BSONObj query_a;
        const std::string query_a_id = args["qid_1"].str();
        if (!helpers::get_one(Collections::QUERIES(), BSON("_id" << query_a_id), query_a)) {
          msg = Error::m(ERR_INVALID_QUERY_ID, query_a_id);
          return false;
        }

        ChromosomeRegionsList regions_a;
        // Load all data from query 1, and intersect.
        // load both region sets.
        if (!retrieve_query(user, args["qid_1"].str(), status, regions_a, msg)) {
          msg = "Cannot retrieve first region set: " + msg;
          return false;
        }

        // Load regions that will be used for the overlap
        ChromosomeRegionsList range_regions;
        const std::string query_b_id = args["qid_2"].str();
        if (!retrieve_query(user, query_b_id, status, range_regions, msg)) {
          msg = "Cannot retrieve second region set: " + msg;
          return false;
        }

        return algorithms::intersect(regions_a, range_regions, regions);
      }

      bool retrieve_overlap_query(const datatypes::User& user,
                                  const mongo::BSONObj &query,
                                  processing::StatusPtr status, ChromosomeRegionsList &regions, std::string &msg)
      {
        processing::RunningOp runningOp = status->start_operation(processing::RETRIEVE_OVERLAP_QUERY, query);
        if (is_canceled(status, msg)) {
          return false;
        }

        mongo::BSONObj args = query["args"].Obj();

        mongo::BSONObj query_a;
        const std::string query_a_id = args["qid_1"].str();
        if (!helpers::get_one(Collections::QUERIES(), BSON("_id" << query_a_id), query_a)) {
          msg = Error::m(ERR_INVALID_QUERY_ID, query_a_id);
          return false;
        }

        ChromosomeRegionsList regions_a;
        // Load all data from query 1, and intersect.
        // load both region sets.
        if (!retrieve_query(user, args["qid_1"].str(), status, regions_a, msg)) {
          msg = "Cannot retrieve first region set: " + msg;
          return false;
        }

        // Load regions that will be used for the overlap
        ChromosomeRegionsList range_regions;
        const std::string query_b_id = args["qid_2"].str();
        if (!retrieve_query(user, query_b_id, status, range_regions, msg)) {
          msg = "Cannot retrieve second region set: " + msg;
          return false;
        }

        const bool overlap = args["overlap"].Bool();
        const double amount = args["amount"].Number();
        const std::string amount_type = args["amount_type"].str();

        return algorithms::overlap(regions_a, range_regions, overlap, amount, amount_type, regions);
      }


      bool retrieve_extend_query(const datatypes::User& user,
                                 const mongo::BSONObj &query,
                                 processing::StatusPtr status, ChromosomeRegionsList &regions, std::string &msg)
      {
        processing::RunningOp runningOp = status->start_operation(processing::RETRIEVE_FLANK_QUERY, query);
        if (is_canceled(status, msg)) {
          return false;
        }

        mongo::BSONObj args = query["args"].Obj();
        const std::string query_id = args["query_id"].str();
        const Length length = args["length"].Int();
        const std::string direction = args["direction"].str();
        const bool use_strand = args["use_strand"].Bool();

        ChromosomeRegionsList query_regions;
        if (!retrieve_query(user, query_id, status, query_regions, msg)) {
          msg = "Cannot retrieve first region set: " + msg;
          return false;
        }

        return algorithms::extend(query_regions, length, direction, use_strand, regions, msg);
      }

      bool retrieve_flank_query(const datatypes::User& user, const mongo::BSONObj &query,
                                processing::StatusPtr status, ChromosomeRegionsList &regions, std::string &msg)
      {
        processing::RunningOp runningOp = status->start_operation(processing::RETRIEVE_FLANK_QUERY, query);
        if (is_canceled(status, msg)) {
          return false;
        }

        mongo::BSONObj args = query["args"].Obj();
        const std::string query_id = args["query_id"].str();
        const Offset start = args["start"].Int();
        const Length length = args["length"].Int();
        const bool use_strand = args["use_strand"].Bool();

        ChromosomeRegionsList query_regions;
        if (!retrieve_query(user, query_id, status, query_regions, msg)) {
          msg = "Cannot retrieve first region set: " + msg;
          return false;
        }

        return algorithms::flank(query_regions, start, length, use_strand, regions, msg);
      }


      bool retrieve_merge_query(const datatypes::User& user,
                                const mongo::BSONObj & query,
                                processing::StatusPtr status, ChromosomeRegionsList & regions, std::string & msg)
      {
        processing::RunningOp runningOp = status->start_operation(processing::RETRIEVE_MERGE_QUERY, query);
        if (is_canceled(status, msg)) {
          return false;
        }

        mongo::BSONObj args = query["args"].Obj();

        // load both region sets.
        ChromosomeRegionsList regions_a;
        bool ret = retrieve_query(user, args["qid_1"].str(), status, regions_a, msg);
        if (!ret) {
          msg = "Cannot retrieve first region set: " + msg;
          return false;
        }

        ChromosomeRegionsList regions_b;
        ret = retrieve_query(user, args["qid_2"].str(), status, regions_b, msg);
        if (!ret) {
          msg = "Cannot retrieve second region set: " + msg;
          return false;
        }

        regions = algorithms::merge_chromosome_regions(regions_a, regions_b);

        return true;
      }


      bool filter_region(const AbstractRegion * region_ref, const std::string & field, const dba::columns::ColumnTypePtr column, Metafield & metafield, const std::string & chrom, algorithms::FilterBuilder::FilterPtr filter, processing::StatusPtr status)
      {
        if (field == "START") {
          return filter->is(region_ref->start());
        }
        if (field == "END") {
          return filter->is(region_ref->end());
        }

        if (field == "CHROMOSOME") {
          return filter->is(chrom);
        }

        // TODO: optimize for "@AGG." values
        if (field[0] == '@') {
          std::string value;
          std::string msg;
          if (!metafield.process(field, chrom, region_ref, status, value, msg)) {
            EPIDB_LOG_ERR(msg);
            return false;
          }
          return filter->is(value);
        } else {
          if (datatypes::column_type_is_compatible(column->type(), datatypes::COLUMN_STRING)) {
            return filter->is(region_ref->get_string(column->pos()));
          } else {
            return filter->is(region_ref->value(column->pos()));
          }
        }
      }

      bool retrieve_filter_query(const datatypes::User& user,
                                 const mongo::BSONObj & query,
                                 processing::StatusPtr status, ChromosomeRegionsList & filtered_regions, std::string & msg)
      {
        processing::RunningOp runningOp = status->start_operation(processing::RETRIEVE_FILTER_QUERY, query);
        if (is_canceled(status, msg)) {
          return false;
        }

        mongo::BSONObj args = query["args"].Obj();

        // load original query
        ChromosomeRegionsList regions;
        bool ret = retrieve_query(user, args["query"].str(), status, regions, msg);
        if (!ret) {
          return false;
        }

        std::string type = args["type"].str();
        std::string operation = args["operation"].str();
        std::string value = args["value"].str();
        std::string field = args["field"].str();

        bool error;
        algorithms::FilterBuilder::FilterPtr filter;

        if (type.compare("string") == 0) {
          filter = algorithms::FilterBuilder::getInstance().build(field, operation, value, error, msg);
          if (error) {
            return false;
          }
        } else if (type.compare("number") == 0 || type.compare("integer") == 0 || type.compare("double") == 0) {
          filter = algorithms::FilterBuilder::getInstance().build(field, operation, atof(value.c_str()), error, msg);
          if (error) {
            return false;
          }
        } else {
          msg = "Invalid type. Valid types are: string, number, integer, double.";
          return false;
        }

        DatasetId dataset_id = -1;
        dba::columns::ColumnTypePtr column;

        size_t total = 0;
        size_t removed = 0;
        size_t keep = 0;

        Metafield metafield;
        for (auto& chromosome_regions_list : regions) {
          const std::string &chromosome = chromosome_regions_list.first;
          Regions saved = Regions();
          for (auto& region : chromosome_regions_list.second) {
            if (!dba::Metafield::is_meta(field)) {
              if (region->dataset_id() != dataset_id) {
                dataset_id = region->dataset_id();
                if (!cache::get_column_type_from_dataset(dataset_id, field, column, msg)) {
                  return false;
                }
              }
              dataset_id = region->dataset_id();
            }
            total++;

            // TODO: use column_type for better filtering. i.e. type conversion
            if (filter_region(region.get(), field, column, metafield, chromosome, filter, status)) {
              saved.emplace_back(std::move(region));
              keep++;
            } else {
              status->subtract_size(region->size());
              status->subtract_regions(1);
              removed++;
            }
          }
          if (!saved.empty()) {
            ChromosomeRegions chr_region(chromosome, std::move(saved));
            filtered_regions.push_back(std::move(chr_region));
          }
        }

        return true;
      }

      bool add_tiling(const std::string & genome, const size_t &tiling_size,
                      DatasetId & dataset_id, std::string & msg)
      {
        Connection c;

        std::string norm_genome = utils::normalize_name(genome);

        auto data_cursor =
          c->query(helpers::collection_name(Collections::TILINGS()),
                   BSON("norm_genome" << norm_genome << "tiling_size" << (int) tiling_size), 0, 1);

        if (!c->getLastError().empty()) {
          msg = c->getLastError();
          c.done();
          return false;
        }
        if (data_cursor->more()) {
          mongo::BSONObj result = data_cursor->next();
          dataset_id = result.getField(KeyMapper::DATASET()).Int();
          c.done();
          return true;
        }

        int t_id;
        if (!helpers::get_increment_counter("tiling", t_id, msg))  {
          c.done();
          return false;
        }
        std::string tiling_id = "tr" + utils::integer_to_string(t_id);

        if (!helpers::get_increment_counter("datasets", dataset_id, msg))  {
          return false;
        }

        mongo::BSONObjBuilder tiling_data_builder;

        std::stringstream name;
        name << "Tiling regions of " << tiling_size << " (Genome " << genome << ")";
        tiling_data_builder.append("_id", tiling_id);
        tiling_data_builder.append(KeyMapper::DATASET(), dataset_id);
        tiling_data_builder.append("name", name.str());
        tiling_data_builder.append("norm_genome", norm_genome);
        tiling_data_builder.append("tiling_size", (int) tiling_size);
        mongo::BSONArrayBuilder ab;
        tiling_data_builder.append("columns", ab.arr());

        c->insert(helpers::collection_name(Collections::TILINGS()), tiling_data_builder.obj());
        if (!c->getLastError().empty()) {
          msg = c->getLastError();
          c.done();
          return false;
        }
        c.done();
        return true;
      }


      bool retrieve_tiling_query(const mongo::BSONObj & query,
                                 processing::StatusPtr status, ChromosomeRegionsList & regions, std::string & msg)
      {
        processing::RunningOp runningOp = status->start_operation(processing::RETRIEVE_TILING_QUERY, query);
        if (is_canceled(status, msg)) {
          return false;
        }

        mongo::BSONObj args = query["args"].Obj();

        const std::string norm_genome = args["norm_genome"].str();
        const size_t tiling_size = args["size"].Int();

        std::vector<std::string> chromosomes;
        if (args.hasField("chromosomes")) {
          chromosomes = utils::build_vector(args["chromosomes"].Array());
        } else {
          std::set<std::string> chrom;
          if (!dba::genomes::get_chromosomes(norm_genome, chrom, msg)) {
            return false;
          }
          chromosomes = std::vector<std::string>(chrom.begin(), chrom.end());
        }

        genomes::GenomeInfoPtr genome_info;
        if (!genomes::get_genome_info(norm_genome, genome_info, msg)) {
          return false;
        }

        std::vector<std::string>::iterator cit;
        dba::genomes::ChromosomeInfo chromosome_info;

        DatasetId tiling_id;
        if (!add_tiling(norm_genome, tiling_size, tiling_id, msg)) {
          return false;
        }

        for (cit = chromosomes.begin(); cit != chromosomes.end(); ++cit) {
          if (!genome_info->get_chromosome(*cit, chromosome_info, msg)) {
            return false;
          }
          Regions regs = Regions();
          for (size_t i = 0; i + tiling_size < chromosome_info.size; i += tiling_size) {
            regs.emplace_back(build_simple_region(i, i + tiling_size, tiling_id));
          }
          regions.push_back(ChromosomeRegions(*cit, std::move(regs)));
        }

        return true;
      }

      bool process_aggregate(const datatypes::User& user,
                             const mongo::BSONObj & query,
                             processing::StatusPtr status, ChromosomeRegionsList & regions, std::string & msg)
      {
        processing::RunningOp runningOp = status->start_operation(processing::PROCESS_AGGREGATE, query);

        if (is_canceled(status, msg)) {
          return false;
        }

        mongo::BSONObj args = query["args"].Obj();
        const std::string query_id = args["data_id"].str();
        const std::string regions_id = args["ranges_id"].str();
        const std::string field = args["field"].str();

        ChromosomeRegionsList data;
        bool ret = retrieve_query(user, query_id, status, data, msg);
        if (!ret) {
          return false;
        }

        ChromosomeRegionsList ranges;
        ret = retrieve_query(user, regions_id, status, ranges, msg);
        if (!ret) {
          return false;
        }

        return algorithms::aggregate(data, ranges, field, status, regions, msg);
      }

      //
      bool get_main_experiment_data(const datatypes::User& user, const std::string &query_id, const std::string field_key,
                                    processing::StatusPtr status, std::vector<std::string>& values, std::string &msg)
      {
        mongo::BSONObj query;
        if (!helpers::get_one(Collections::QUERIES(), BSON("_id" << query_id), query)) {
          msg = Error::m(ERR_INVALID_QUERY_ID, query_id);
          return false;
        }

        const std::string& type = query["type"].str();
        const mongo::BSONObj& args = query["args"].Obj();

        if (((field_key == "norm_genome") || (field_key == "genome")) && args.hasField("norm_genome")) {
          values.emplace_back( args.getFieldDotted("norm_genome").String() );
          return true;
        }

        if (type == "experiment_select") {
          std::vector<std::string> norm_names = utils::build_vector(args["norm_experiment_name"].Array());
          if (!norm_names.empty()) {
            for(const auto& exp_name: norm_names) {
              mongo::BSONObj exp_obj;
              if (!dba::experiments::by_name(exp_name, exp_obj, msg)) {
                return false;
              }
              auto value = exp_obj.getFieldDotted(field_key).String();
              values.push_back(value);
            }

            return true;
          }

          if (args.hasField(field_key)) {
            if (args[field_key].type() == mongo::Array) {
              values = utils::build_vector(args[field_key].Array());
            } else {
              values.push_back(args[field_key].String());
            }
          }

        } else if (type == "annotation_select") {

          std::vector<std::string> ann_names = utils::build_vector(args["annotation"].Array());

          const std::string genome = args["genome"].String();

          for (const auto& ann_name: ann_names) {
            mongo::BSONObj ann_obj;
            if(!dba::annotations::by_name(ann_name, genome, ann_obj, msg)) {
              return false;
            }

            if (ann_obj.hasField(field_key)) {
              auto value = ann_obj.getFieldDotted(field_key).String();
              values.push_back(value);
            }
          }

          return true;

        } else if (type == "intersect") {
          const std::string query_id = args["qid_1"].str();
          return get_main_experiment_data(user, query_id, field_key, status, values, msg);

        } else if (type == "overlap") {
          const std::string query_id = args["qid_1"].str();
          return get_main_experiment_data(user, query_id, field_key, status, values, msg);

        } else if (type == "flank") {
          const std::string query_id = args["query_id"].str();
          return get_main_experiment_data(user, query_id, field_key, status, values, msg);

        } else if (type == "extend") {
          const std::string query_id = args["query_id"].str();
          return get_main_experiment_data(user, query_id, field_key, status, values, msg);

        } else if (type == "merge") {
          const std::string query_id = args["qid_1"].str();
          return get_main_experiment_data(user, query_id, field_key, status, values, msg);

        } else if (type == "genes_select") {
          if (args.hasElement("genes")) {
            auto genes = utils::build_vector(args["genes"].Array());
            values.insert(values.end(), genes.begin(), genes.end());
          }

          std::vector<std::string> go_terms;
          if (args.hasElement("go_terms")) {
            go_terms = utils::build_vector(args["go_terms"].Array());
            values.insert(values.end(), go_terms.begin(), go_terms.end());
          }

          // Honestly I dont like it, but since we changed these parameters and we already have a database with queries...
          // TODO: manually change the database.
          std::string gene_model;
          if (args.hasField("gene_set")) {
            gene_model = args["gene_set"].str();
          } else {
            gene_model = args["gene_model"].str();
          }
          values.push_back(gene_model);
          return true;

        } else if (type == "find_motif") {
          std::string name = args["motif"].String() + " (motif)";
          values.push_back(name);
          return true;

        } else if (type == "expressions_select") {
          if (args.hasElement("genes")) {
            auto genes = utils::build_vector(args["genes"].Array());
            values.insert(values.end(), genes.begin(), genes.end());
          }

        } else if (type == "filter") {
          const std::string query_id = args["query"].str();
          return get_main_experiment_data(user, query_id, field_key, status, values, msg);


        } else if (type == "tiling") {
          std::string name = utils::integer_to_string(args["size"].Int()) + " (tiling regions)";
          values.push_back(name);

        } else if (type == "aggregate") {
          const std::string query_id = args["query_id"].str();
          return get_main_experiment_data(user, query_id, field_key, status, values, msg);

        } else if (type == "input_regions") {
          values.push_back("input_regions");
          return true;

        } else {
          msg = Error::m(ERR_UNKNOW_QUERY_TYPE, type);
          return false;
        }

        msg = "It must not be here";
        return false;
      }


      // TODO: move to another file
      bool __get_columns_from_dataset(const DatasetId & dataset_id, std::vector<mongo::BSONObj> &columns, std::string & msg)
      {
        if (dataset_id == DATASET_EMPTY_ID) {
          return true;
        }

        mongo::BSONObj o = BSON(KeyMapper::DATASET() << dataset_id);

        Connection c;
        auto cursor = c->query(helpers::collection_name(Collections::EXPERIMENTS()), o);

        bool found = false;
        while (cursor->more()) {
          mongo::BSONObj experiment = cursor->next();
          int s_count = 0;
          int n_count = 0;

          std::vector<mongo::BSONElement> tmp_columns = experiment["columns"].Array();
          for (const mongo::BSONElement & e : tmp_columns) {
            mongo::BSONObj column = e.Obj();
            const std::string &column_type = column["column_type"].str();
            const std::string &column_name = column["name"].str();

            mongo::BSONObjBuilder bob;
            bob.appendElements(column);

            if (column_name != "CHROMOSOME" && column_name != "START" &&  column_name != "END") {
              int pos = -1;
              if (column_type == "string") {
                pos = s_count++;
              } else if (column_type == "integer") {
                pos = n_count++;
              } else if (column_type == "double") {
                pos = n_count++;
              } else if (column_type == "range") {
                pos = n_count++;
              } else if (column_type == "category") {
                pos = s_count++;
              } else if (column_type == "calculated") {
                msg = "Calculated field does not have data";
                return false;
              } else {
                msg = "Unknown column type: " + column_type;
                return false;
              }
              bob.append("pos", pos);
            }

            mongo::BSONObj o = bob.obj();
            columns.push_back(o);
          }
          found = true;
        }

        if (found) {
          c.done();
          return true;
        }

        cursor = c->query(helpers::collection_name(Collections::ANNOTATIONS()), o);
        while (cursor->more()) {
          mongo::BSONObj annotation = cursor->next().getOwned();
          int s_count = 0;
          int n_count = 0;
          std::vector<mongo::BSONElement> tmp_columns = annotation["columns"].Array();
          for (const mongo::BSONElement & e : tmp_columns) {
            mongo::BSONObj column = e.Obj();
            const std::string &column_type = column["column_type"].str();
            const std::string &column_name = column["name"].str();

            mongo::BSONObjBuilder bob;
            bob.appendElements(column);

            if (column_name != "CHROMOSOME" && column_name != "START" &&  column_name != "END") {
              int pos = -1;
              if (column_type == "string") {
                pos = s_count++;
              } else if (column_type == "integer") {
                pos = n_count++;
              } else if (column_type == "double") {
                pos = n_count++;
              } else if (column_type == "range") {
                pos = n_count++;
              } else if (column_type == "category") {
                pos = s_count++;
              } else if (column_type == "calculated") {
                msg = "Calculated field does not have data";
                return false;
              } else {
                msg = "Unknown column type: " + column_type;
                return false;
              }
              bob.append("pos", pos);
            }
            columns.push_back(bob.obj());
          }
          found = true;
        }

        if (found) {
          c.done();
          return true;
        }

        cursor = c->query(helpers::collection_name(Collections::TILINGS()), o);
        while (cursor->more()) {
          mongo::BSONObj tiling = cursor->next().getOwned();
          std::vector<mongo::BSONElement> tmp_columns = tiling["columns"].Array();
          for (const mongo::BSONElement & e : tmp_columns) {
            columns.push_back(e.Obj().getOwned());
          }
          c.done();
          return true;
        }

        cursor = c->query(helpers::collection_name(Collections::QUERIES()), BSON("type" << "input_regions" << "args.dataset_id" << dataset_id));
        if (cursor->more()) {
          for (const auto& column : parser::FileFormat::default_format()) {
            columns.push_back(column->BSONObj());
          }
          c.done();
          return true;
        }

        cursor = c->query(helpers::collection_name(Collections::GENE_MODELS()), o);
        if (cursor->more()) {
          int pos = 0;
          for (const auto& column : parser::FileFormat::gtf_format()) {
            mongo::BSONObjBuilder bob;
            const std::string &column_name = column->name();
            if (column_name != "CHROMOSOME" && column_name != "START" &&  column_name != "END") {
              bob.appendElements(column->BSONObj());
              bob.append("pos", pos++);
              columns.emplace_back(bob.obj());
            }
          }
          found = true;
        }

        cursor = c->query(helpers::collection_name(Collections::GENE_EXPRESSIONS()), o);
        if (cursor->more()) {
          mongo::BSONObj gene_expression = cursor->next().getOwned();
          int s_count = 0;
          int n_count = 0;
          std::vector<mongo::BSONElement> tmp_columns = gene_expression["columns"].Array();
          for (const mongo::BSONElement & e : tmp_columns) {
            mongo::BSONObj column = e.Obj();
            const std::string &column_type = column["column_type"].str();
            const std::string &column_name = column["name"].str();
            mongo::BSONObjBuilder bob;
            if (column_name != "CHROMOSOME" && column_name != "START" &&  column_name != "END") {
              int pos = -1;
              if (column_type == "string") {
                pos = s_count++;
              } else if (column_type == "integer") {
                pos = n_count++;
              } else if (column_type == "double") {
                pos = n_count++;
              }
              bob.appendElements(column);
              bob.append("pos", pos);
              columns.emplace_back(bob.obj());
            }
          }
          found = true;
        }


        c.done();
        if (found) {
          return true;
        } else {
          msg = Error::m(ERR_DATASET_NOT_FOUND, dataset_id);
          return false;
        }
      }


      bool __get_bson_by_dataset_id(DatasetId dataset_id, const std::string collection, mongo::BSONObj &obj)
      {
        Connection c;
        auto data_cursor = c->query(helpers::collection_name(collection), mongo::Query(BSON(KeyMapper::DATASET() << dataset_id)));
        if (data_cursor->more()) {
          obj = data_cursor->next().getOwned();
          c.done();
          return true;
        }
        c.done();
        return false;
      }

      bool __get_bson_by_dataset_id(DatasetId dataset_id, mongo::BSONObj &obj, std::string &msg)
      {
        if (__get_bson_by_dataset_id(dataset_id, Collections::EXPERIMENTS(), obj)) {
          return true;
        }

        if (__get_bson_by_dataset_id(dataset_id, Collections::ANNOTATIONS(), obj)) {
          return true;
        }

        if (__get_bson_by_dataset_id(dataset_id, Collections::GENE_MODELS(), obj)) {
          return true;
        }

        if (__get_bson_by_dataset_id(dataset_id, Collections::GENE_EXPRESSIONS(), obj)) {
          return true;
        }

        if (__get_bson_by_dataset_id(dataset_id, Collections::TILINGS(), obj)) {
          return true;
        }

        Connection c;
        auto data_cursor = c->query(helpers::collection_name(Collections::QUERIES()),
                                    BSON("type" << "input_regions" << "args.dataset_id" << dataset_id));
        if (data_cursor->more()) {
          mongo::BSONObj query_obj = data_cursor->next().getOwned();
          obj = BSON("name" << ("Query " + query_obj["_id"].String() + " regions set") <<
                     "norm_genome" << query_obj["args"]["norm_genome"].String()
                    );
          c.done();
          return true;
        }
        c.done();

        msg = Error::m(ERR_DATASET_NOT_FOUND, dataset_id);
        return false;
      }

      // TODO: Move to processing file
      bool is_canceled(processing::StatusPtr status, std::string& msg)
      {
        bool is_canceled = false;
        if (!status->is_canceled(is_canceled, msg)) {
          return true;
        }
        if (is_canceled) {
          msg = Error::m(ERR_REQUEST_CANCELED);
          return true;
        }
        return false;
      }
    }
  }
}
