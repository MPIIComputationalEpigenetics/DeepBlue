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
#include <utility>

#include <mongo/bson/bson.h>

#include "../algorithms/algorithms.hpp"
#include "../algorithms/filter.hpp"

#include "../connection/connection.hpp"

#include "../datatypes/column_types_def.hpp"
#include "../datatypes/regions.hpp"

#include "../dba/experiments.hpp"

#include "../extras/utils.hpp"

#include "collections.hpp"
#include "dba.hpp"
#include "genes.hpp"
#include "genomes.hpp"
#include "helpers.hpp"
#include "key_mapper.hpp"
#include "metafield.hpp"
#include "retrieve.hpp"
#include "users.hpp"

#include "queries.hpp"
#include "queries_cache.hpp"

#include "../errors.hpp"
#include "../log.hpp"

namespace epidb {
  namespace dba {
    namespace query {

      // TODO: Merge wtth users cache and use templates
      class ExperimentNameDatasetIdCache {
      private:

        std::map<std::string, DatasetId> name_dataset_id;

      public:

        DatasetId get_dataset_id(const std::string &name)
        {
          return name_dataset_id[name];
        }

        void set_dataset_id(const std::string &name, const DatasetId id)
        {
          name_dataset_id[name] = id;
        }

        bool exists_dataset_id(const std::string &name)
        {
          if (name_dataset_id.find(name) != name_dataset_id.end()) {
            return true;
          }
          return false;
        }

        void invalidate()
        {
          name_dataset_id.clear();
        }
      };

      ExperimentNameDatasetIdCache experiment_name_dataset_id_cache;


      void invalidate_cache()
      {
        experiment_name_dataset_id_cache.invalidate();
      }


      bool store_query(const std::string &type, const mongo::BSONObj &args, const std::string &user_key,
                       std::string &query_id, std::string &msg)
      {
        utils::IdName user;
        if (!users::get_user(user_key, user, msg)) {
          return false;
        }

        mongo::BSONObjBuilder search_query_builder;
        search_query_builder.append("type", type);
        search_query_builder.append("args", args);
        search_query_builder.append("user", user.id);
        mongo::BSONObj search_query = search_query_builder.obj();

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
        stored_query_builder.append("user", user.id);
        stored_query_builder.appendTimeT("time", time_);
        stored_query_builder.append("type", type);
        stored_query_builder.append("args", args);

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

      bool modify_query(const std::string &query_id, const std::string &key, const std::string &value, const std::string &user_key,
                        std::string &new_query_id, std::string &msg)
      {
        mongo::BSONObj old_query;
        if (!helpers::get_one(Collections::QUERIES(), BSON("_id" << query_id), old_query)) {
          msg = Error::m(ERR_INVALID_QUERY_ID, query_id);
          return false;
        }

        datatypes::User user;
        if (!users::get_user_by_key(user_key, user, msg)) {
          return false;
        }

        if (old_query["user"].String() != user.get_id()) {
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
        search_query_builder.append("user", user.get_id());
        search_query_builder.append("type", old_query["type"].String());
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
        stored_query_builder.append("user", user.get_id());
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

      bool retrieve_query(const std::string &user_key, const std::string &query_id,
                          processing::StatusPtr status, ChromosomeRegionsList &regions, std::string &msg)
      {
        mongo::BSONObj query;
        if (!helpers::get_one(Collections::QUERIES(), BSON("_id" << query_id), query)) {
          msg = Error::m(ERR_INVALID_QUERY_ID, query_id);
          return false;
        }

        const std::string& type = query["type"].str();

        const mongo::BSONObj& args = query["args"].Obj();
        if (args.hasField("cache") && args["cache"].String() == "yes") {
          return cache::get_query_cache(user_key, query["derived_from"].String(), status, regions, msg);
        }

        if (type == "experiment_select") {
          if (!retrieve_experiment_select_query(query, status, regions, msg)) {
            return false;
          }
        } else if (type == "intersect") {
          if (!retrieve_intersection_query(user_key, query, status, regions, msg)) {
            return false;
          }
        } else if (type == "flank") {
          if (!retrieve_flank_query(user_key, query, status, regions, msg)) {
            return false;
          }
        } else if (type == "merge") {
          if (!retrieve_merge_query(user_key, query, status, regions, msg)) {
            return false;
          }
        } else if (type == "annotation_select") {
          if (!retrieve_annotation_select_query(user_key, query, status, regions, msg)) {
            return false;
          }
        } else if (type == "genes_select") {
          if (!retrieve_genes_select_query(user_key, query, status, regions, msg)) {
            return false;
          }
        } else if (type == "filter") {
          if (!retrieve_filter_query(user_key, query, status, regions, msg)) {
            return false;
          }
        } else if (type == "tiling") {
          if (!retrieve_tiling_query(query, status, regions, msg)) {
            return false;
          }
        } else if (type == "aggregate") {
          if (!process_aggregate(user_key, query, status, regions, msg)) {
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

      /*
      bool get_possible_experiments_by_query(const std::string &user_key, const std::string &query_id,
                                    processing::StatusPtr status, std::vector<utils::IdName> &experiments_name, std::string &msg) {


                mongo::BSONObj query;
        if (!helpers::get_one(Collections::QUERIES(), BSON("_id" << query_id), query)) {
          msg = Error::m(ERR_INVALID_QUERY_ID, query_id);
          return false;
        }

        const std::string& type = query["type"].str();

        const mongo::BSONObj& args = query["args"].Obj();
        if (args.hasField("cache") && args["cache"].String() == "yes") {
          return cache::get_query_cache(user_key, query["derived_from"].String(), status, regions, msg);
        }

        if (type == "experiment_select") {
          // get genome directly
        } else if (type == "annotation_select") {
          // get genome directly
        } else if (type == "genes_select") {
          // get genome from gene set
        } else if (type == "tiling") {
          // get genome directly
        } else if (type == "input_regions") {
          // get genome directly
        } else {

        get args
       qid_1
       qid_2
       query_id
        }

      }

      */

      bool get_experiments_by_query(const std::string &user_key, const std::string &query_id,
                                    processing::StatusPtr status, std::vector<utils::IdName> &experiments_name, std::string &msg)
      {
        processing::RunningOp runningOp = status->start_operation(processing::GET_EXPERIMENT_BY_QUERY);
        if (is_canceled(status, msg)) {
          return false;
        }

        ChromosomeRegionsList chromossome_regions;
        if (!retrieve_query(user_key, query_id, status, chromossome_regions, msg)) {
          return false;
        }

        std::set<DatasetId> datasets_it_set;
        for (auto  &regions : chromossome_regions) {
          for (auto &region : regions.second) {
            datasets_it_set.insert(region->dataset_id());
          }
        }

        std::vector<DatasetId> datasets_it(datasets_it_set.size());
        std::copy(datasets_it_set.begin(), datasets_it_set.end(), datasets_it.begin());

        mongo::BSONObjBuilder experiments_query_builder;
        experiments_query_builder << KeyMapper::DATASET() << helpers::build_condition_array<DatasetId>(datasets_it, "$in");

        mongo::BSONObj o = experiments_query_builder.obj();

        Connection c;
        auto cursor = c->query(helpers::collection_name(Collections::EXPERIMENTS()), o);
        while (cursor->more()) {
          mongo::BSONObj experiment = cursor->next();
          std::string exp_id = experiment["_id"].str();
          std::string exp_name = experiment["name"].str();
          utils::IdName p(exp_id, exp_name);
          experiments_name.push_back(p);
        }
        cursor = c->query(helpers::collection_name(Collections::ANNOTATIONS()), o);
        while (cursor->more()) {
          mongo::BSONObj experiment = cursor->next();
          std::string exp_id = experiment["_id"].str();
          std::string exp_name = experiment["name"].str();
          utils::IdName p(exp_id, exp_name);
          experiments_name.push_back(p);
        }
        cursor = c->query(helpers::collection_name(Collections::TILINGS()), o);
        while (cursor->more()) {
          mongo::BSONObj experiment = cursor->next();
          std::string exp_id = experiment["_id"].str();
          std::string exp_name = experiment["name"].str();
          utils::IdName p(exp_id, exp_name);
          experiments_name.push_back(p);
        }

        c.done();
        return true;
      }


      bool count_regions(const std::string &query_id, const std::string &user_key,
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
        if (result.size() == 0) {
          msg = "Query key is invalid";
          return false;
        }
        mongo::BSONObj query = result[0];
        mongo::BSONObj args = query["args"].Obj();

        count = 0;
        ChromosomeRegionsList regions;
        if (!retrieve_query(user_key, query_id, status, regions, msg)) {
          return false;
        }

        count = 0;
        for (const auto& chromosome : regions) {
          count += chromosome.second.size();
        }

        return true;
      }

      const mongo::BSONObj build_query(const mongo::BSONObj &args)
      {
        mongo::BSONObjBuilder regions_query_builder;

        // Get the experiments
        mongo::BSONObjBuilder experiments_query_builder;

        if (args.hasField("genome")) {
          if (args["genome"].type() == mongo::Array) {
            experiments_query_builder.append("norm_genome", BSON("$in" << args["norm_genome"]));
          } else {
            experiments_query_builder.append("norm_genome", args["norm_genome"].str());
          }
        }
        if (args.hasField("experiment_name")) {
          if (args["experiment_name"].type() == mongo::Array) {
            experiments_query_builder.append("norm_name", BSON("$in" << args["norm_experiment_name"]));
          } else {
            experiments_query_builder.append("norm_name", args["norm_experiment_name"].str());
          }
        }
        if (args.hasField("epigenetic_mark")) {
          if (args["epigenetic_mark"].type() == mongo::Array) {
            experiments_query_builder.append("norm_epigenetic_mark", BSON("$in" << args["norm_epigenetic_mark"]));
          } else {
            experiments_query_builder.append("norm_epigenetic_mark", args["norm_epigenetic_mark"].str());
          }
        }
        if (args.hasField("sample_id")) {
          if (args["sample_id"].type() == mongo::Array) {
            experiments_query_builder.append("sample_id", BSON("$in" << args["sample_id"]));
          } else {
            experiments_query_builder.append("sample_id", args["sample_id"].str());
          }
        }
        if (args.hasField("project")) {
          if (args["project"].type() == mongo::Array) {
            experiments_query_builder.append("norm_project", BSON("$in" << args["norm_project"]));
          } else {
            experiments_query_builder.append("norm_project", args["norm_project"].str());
          }
        }
        if (args.hasField("technique")) {
          if (args["technique"].type() == mongo::Array) {
            experiments_query_builder.append("norm_technique", BSON("$in" << args["norm_technique"]));
          } else {
            experiments_query_builder.append("norm_technique", args["norm_technique"].str());
          }
        }
        if (args.hasField("sample_info.biosource_name")) {
          if (args["sample_info.biosource_name"].type() == mongo::Array) {
            experiments_query_builder.append("sample_info.norm_biosource_name", BSON("$in" << args["sample_info.norm_biosource_name"]));
          } else {
            experiments_query_builder.append("sample_info.norm_biosource_name", args["sample_info.norm_biosource_name"].str());
          }
        }
        if (args.hasField("upload_info.content_format")) {
          if (args["upload_info.content_format"].type() == mongo::Array) {
            experiments_query_builder.append("upload_info.content_format", BSON("$in" << args["upload_info.content_format"]));
          } else {
            experiments_query_builder.append("upload_info.content_format", args["upload_info.content_format"].str());
          }
        }
        if (args.hasField("upload_info.upload_end")) {
          experiments_query_builder.append(args["upload_info.upload_end"]);
        }
        experiments_query_builder.append("upload_info.done", true);
        return experiments_query_builder.obj();
      }

      bool build_experiment_query(const int start, const int end, const std::string &experiment_name,
                                  mongo::BSONObj &regions_query, std::string &msg)
      {
        Connection c;

        DatasetId dataset_id;
        std::string norm_name = utils::normalize_name(experiment_name);
        if (experiment_name_dataset_id_cache.exists_dataset_id(norm_name)) {
          dataset_id = experiment_name_dataset_id_cache.get_dataset_id(norm_name);
        } else {
          auto cursor =
            c->query(helpers::collection_name(Collections::EXPERIMENTS()), BSON("norm_name" << norm_name));

          if (!cursor->more()) {
            msg = "Experiments name '" + experiment_name + "' not found";
            c.done();
            return false;
          }

          mongo::BSONObj p = cursor->next();
          mongo::BSONElement dataset_id_elem = p.getField(KeyMapper::DATASET());
          dataset_id = dataset_id_elem.Int();
          experiment_name_dataset_id_cache.set_dataset_id(norm_name, dataset_id);
        }

        mongo::BSONObjBuilder regions_query_builder;
        regions_query_builder.append(KeyMapper::DATASET(), dataset_id);
        regions_query_builder.append(KeyMapper::START(), BSON("$lte" << end));
        regions_query_builder.append(KeyMapper::END(), BSON("$gte" << start));

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

      bool build_experiment_query(const mongo::BSONObj &query,
                                  mongo::BSONObj &regions_query, std::string &msg)
      {
        mongo::BSONObj args = query["args"].Obj();

        mongo::BSONArrayBuilder datasets_array_builder;

        const mongo::BSONObj args_query = build_query(args);
        Connection c;
        auto cursor = c->query(helpers::collection_name(Collections::EXPERIMENTS()), args_query);
        while (cursor->more()) {
          mongo::BSONObj p = cursor->next();
          mongo::BSONElement dataset_id = p.getField(KeyMapper::DATASET());
          datasets_array_builder.append(dataset_id.Int());
        }
        c.done();

        mongo::BSONArray datasets_array = datasets_array_builder.arr();

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

        Connection c;
        mongo::BSONObj annotation_query = annotations_query_builder.obj();
        auto cursor = c->query(helpers::collection_name(Collections::ANNOTATIONS()), annotation_query);
        while (cursor->more()) {
          mongo::BSONObj p = cursor->next();
          mongo::BSONElement dataset_id = p.getField(KeyMapper::DATASET());
          datasets_array_builder.append(dataset_id.Int());
        }
        c.done();

        mongo::BSONArray datasets_array = datasets_array_builder.arr();
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

      bool retrieve_experiment_select_query(const mongo::BSONObj &query,
                                            processing::StatusPtr status, ChromosomeRegionsList &regions, std::string &msg)
      {
        processing::RunningOp runningOp = status->start_operation(processing::RETRIEVE_EXPERIMENT_SELECT_QUERY, query);
        if (is_canceled(status, msg)) {
          return false;
        }

        mongo::BSONObj regions_query;
        if (!build_experiment_query(query, regions_query, msg)) {
          return false;
        }

        mongo::BSONObj args = query["args"].Obj();

        std::vector<std::string> chromosomes = utils::build_vector(args["chromosomes"].Array());
        std::set<std::string> genomes = utils::build_set(args["norm_genomes"].Array());

        std::vector<ChromosomeRegionsList> genome_regions;

        // get region data for all genomes
        for (const auto& genome : genomes) {
          ChromosomeRegionsList reg;
          if (!retrieve::get_regions(genome, chromosomes, regions_query, true, status, reg, msg)) {
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

        if (!retrieve::get_regions(genome, chromosomes, regions_query, true, status, regions, msg)) {
          return false;
        }

        return true;
      }

      bool retrieve_annotation_select_query(const std::string &user_key, const mongo::BSONObj &query,
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
        std::vector<std::string> chromosomes = utils::build_vector(args["chromosomes"].Array());

        std::vector<ChromosomeRegionsList> genome_regions;

        // get region data for all genomes
        std::vector<mongo::BSONElement> genome_arr = args["norm_genomes"].Array();
        std::vector<mongo::BSONElement>::iterator git;
        for (git = genome_arr.begin(); git != genome_arr.end(); ++git) {
          ChromosomeRegionsList reg;
          if (!retrieve::get_regions(git->str(), chromosomes, regions_query, true, status, reg, msg)) {
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

      bool retrieve_genes_select_query(const std::string &user_key, const mongo::BSONObj &query,
                                       processing::StatusPtr status, ChromosomeRegionsList &regions, std::string &msg)
      {
        processing::RunningOp runningOp = status->start_operation(processing::RETRIEVE_GENES_DATA, query);
        if (is_canceled(status, msg)) {
          return false;
        }

        mongo::BSONObj args = query["args"].Obj();
        const std::vector<std::string> genes = utils::build_vector(args["genes"].Array());
        const std::string gene_set = args["gene_set"].str();


        if (!genes::get_genes_from_database(genes, gene_set, regions, msg)) {
          return false;
        }

        return true;
      }


      bool retrieve_intersection_query(const std::string &user_key, const mongo::BSONObj &query,
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
        std::string query_a_type = query_a["type"].str();
        bool use_fast = false;
        //if ((query_a_type == "experiment_select") || (query_a_type == "annotation_select")) {
        if (query_a_type == "experiment_select") {
          use_fast = true;
        }

        // Load regions that will be used for the overlap
        ChromosomeRegionsList range_regions;
        const std::string query_b_id = args["qid_2"].str();
        if (!retrieve_query(user_key, query_b_id, status, range_regions, msg)) {
          msg = "Cannot retrieve second region set: " + msg;
          return false;
        }

        ChromosomeRegionsList regions_a;

        // The "fast" is in fact very slow!
        use_fast = false;
        if (use_fast) {
          mongo::BSONObj args = query_a["args"].Obj();

          mongo::BSONArrayBuilder datasets_array_builder;
          const mongo::BSONObj query = build_query(args);
          Connection c;
          auto cursor = c->query(helpers::collection_name(Collections::EXPERIMENTS()), query);
          while (cursor->more()) {
            mongo::BSONObj p = cursor->next();
            mongo::BSONElement dataset_id = p.getField(KeyMapper::DATASET());
            datasets_array_builder.append(dataset_id.Int());
          }
          c.done();

          mongo::BSONArray datasets_array = datasets_array_builder.arr();

          std::set<std::string> genomes = utils::build_set(args["norm_genomes"].Array());

          std::vector<ChromosomeRegionsList> genome_regions;

          unsigned int selection_start = std::numeric_limits<unsigned int>::min();
          unsigned int selection_end = std::numeric_limits<unsigned int>::max();

          if (args.hasField("start")) {
            selection_start = args["start"].Int();
          }

          if (args.hasField("end")) {
            selection_end = args["end"].Int();
          }

          // get region data for all genomes
          for (const auto& genome : genomes) {
            ChromosomeRegionsList chromosome_region_list;
            for (auto &chromosome : range_regions) {
              Regions chromosome_regions = Regions();
              for (auto &region : chromosome.second) {
                if (region->start() >= selection_start && region->end() <= selection_end) {
                  mongo::BSONObj regions_query;
                  if (!dba::query::build_experiment_query(region->start(), region->end(), datasets_array, regions_query, msg)) {
                    return false;
                  }

                  Regions regions;
                  if (!dba::retrieve::get_regions(genome, chromosome.first, regions_query, false, status, regions, msg)) {
                    return false;
                  }

                  chromosome_regions.reserve(chromosome_regions.size() + regions.size());
                  chromosome_regions.insert(chromosome_regions.end(),
                                            std::make_move_iterator(regions.begin()),
                                            std::make_move_iterator(regions.end()));
                }
              }
              ChromosomeRegions chromossomeRegions(chromosome.first, std::move(chromosome_regions));
              chromosome_region_list.push_back(std::move(chromossomeRegions));
            }
            genome_regions.push_back(std::move(chromosome_region_list));
          }

          // merge region data of all genomes
          auto rit = genome_regions.begin();
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

          // Do the slow way: load all data from query 1, and intersect.
        } else {
          // load both region sets.

          if (!retrieve_query(user_key, args["qid_1"].str(), status, regions_a, msg)) {
            msg = "Cannot retrieve first region set: " + msg;
            return false;
          }

          return algorithms::intersect(regions_a, range_regions, regions);

          return true;
        }
      }

      bool retrieve_flank_query(const std::string &user_key, const mongo::BSONObj &query,
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
        if (!retrieve_query(user_key, query_id, status, query_regions, msg)) {
          msg = "Cannot retrieve first region set: " + msg;
          return false;
        }

        return algorithms::flank(query_regions, start, length, use_strand, regions, msg);
      }

      bool retrieve_merge_query(const std::string & user_key, const mongo::BSONObj & query,
                                processing::StatusPtr status, ChromosomeRegionsList & regions, std::string & msg)
      {
        processing::RunningOp runningOp = status->start_operation(processing::RETRIEVE_MERGE_QUERY, query);
        if (is_canceled(status, msg)) {
          return false;
        }

        mongo::BSONObj args = query["args"].Obj();

        // load both region sets.
        ChromosomeRegionsList regions_a;
        bool ret = retrieve_query(user_key, args["qid_1"].str(), status, regions_a, msg);
        if (!ret) {
          msg = "Cannot retrieve first region set: " + msg;
          return false;
        }

        ChromosomeRegionsList regions_b;
        ret = retrieve_query(user_key, args["qid_2"].str(), status, regions_b, msg);
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
          return filter->is(region_ref->value(column->pos()));
        }
      }

      bool retrieve_filter_query(const std::string & user_key, const mongo::BSONObj & query,
                                 processing::StatusPtr status, ChromosomeRegionsList & filtered_regions, std::string & msg)
      {
        processing::RunningOp runningOp = status->start_operation(processing::RETRIEVE_FILTER_QUERY, query);
        if (is_canceled(status, msg)) {
          return false;
        }

        mongo::BSONObj args = query["args"].Obj();

        // load original query
        ChromosomeRegionsList regions;
        bool ret = retrieve_query(user_key, args["query"].str(), status, regions, msg);
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
                if (!dba::experiments::get_field_pos(dataset_id, field, column, msg)) {
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
              removed++;
            }
          }
          ChromosomeRegions chr_region(chromosome, std::move(saved));
          filtered_regions.push_back(std::move(chr_region));
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
        tiling_data_builder.append("genome", genome);
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

        const std::string genome = args["genome"].str();
        const std::string norm_genome = args["norm_genome"].str();
        const size_t tiling_size = args["size"].Int();

        std::vector<std::string> chromosomes;
        std::vector<mongo::BSONElement> chr_arr = args["chromosomes"].Array();
        std::vector<mongo::BSONElement>::iterator it;
        for (it = chr_arr.begin(); it != chr_arr.end(); ++it) {
          chromosomes.push_back(it->str());
        }

        genomes::GenomeInfoPtr genome_info;
        if (!genomes::get_genome_info(norm_genome, genome_info, msg)) {
          return false;
        }

        std::vector<std::string>::iterator cit;
        dba::genomes::ChromosomeInfo chromosome_info;

        DatasetId tiling_id;
        if (!add_tiling(genome, tiling_size, tiling_id, msg)) {
          return false;
        }

        for (cit = chromosomes.begin(); cit != chromosomes.end(); ++cit) {
          if (!genome_info->get_chromosome(*cit, chromosome_info, msg)) {
            msg = "Chromosome " + *cit + " does not exist on genome " + genome + ".";
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

      bool process_aggregate(const std::string & user_key, const mongo::BSONObj & query,
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
        bool ret = retrieve_query(user_key, query_id, status, data, msg);
        if (!ret) {
          return false;
        }

        ChromosomeRegionsList ranges;
        ret = retrieve_query(user_key, regions_id, status, ranges, msg);
        if (!ret) {
          return false;
        }

        return algorithms::aggregate(data, ranges, field, status, regions, msg);
      }

      bool get_columns_from_dataset(const DatasetId & dataset_id, std::vector<mongo::BSONObj> &columns, std::string & msg)
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
          for(const mongo::BSONElement & e: tmp_columns) {
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
          for(const mongo::BSONElement & e: tmp_columns) {
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
          for(const mongo::BSONElement & e: tmp_columns) {
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

        cursor = c->query(helpers::collection_name(Collections::GENE_SETS()), o);
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

        c.done();
        if (found) {
          return true;
        } else {
          msg = Error::m(ERR_DATASET_NOT_FOUND, dataset_id);
          return false;
        }
      }

      bool is_canceled(processing::StatusPtr status, std::string msg)
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
