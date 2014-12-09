//
//  queries.cpp
//  epidb
//
//  Created by Felipe Albrecht on 09.07.13.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include <ctime>
#include <cstring>

#include <boost/foreach.hpp>

#include <mongo/bson/bson.h>
#include <mongo/client/dbclient.h>

#include "../errors.hpp"
#include "../regions.hpp"
#include "../log.hpp"

#include "../algorithms/aggregate.hpp"
#include "../algorithms/intersection.hpp"
#include "../algorithms/merge.hpp"

#include "../datatypes/column_types_def.hpp"

#include "../dba/experiments.hpp"

#include "../extras/utils.hpp"

#include "collections.hpp"
#include "config.hpp"
#include "dba.hpp"
#include "filter.hpp"
#include "genomes.hpp"
#include "helpers.hpp"
#include "key_mapper.hpp"
#include "metafield.hpp"
#include "queries.hpp"
#include "retrieve.hpp"
#include "users.hpp"

namespace epidb {
  namespace dba {
    namespace query {

      bool store_query(const std::string &type, const mongo::BSONObj &args, const std::string &user_key,
                       std::string &query_id, std::string &msg)
      {
        int query_counter;
        if (!helpers::get_counter("queries", query_counter, msg)) {
          return false;
        }
        query_id = "q" + utils::integer_to_string(query_counter);
        time_t time_;
        time(&time_);

        mongo::BSONObjBuilder stored_query_builder;

        stored_query_builder.append("_id", query_id);
        std::string user_name;
        if (!users::get_user_name(user_key, user_name, msg)) {
          return false;
        }
        stored_query_builder.append("user", user_name);
        stored_query_builder.appendTimeT("time", time_);
        stored_query_builder.append("type", type);
        stored_query_builder.append("args", args);

        mongo::ScopedDbConnection c(config::get_mongodb_server());

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
                          ChromosomeRegionsList &regions, std::string &msg)
      {
        std::vector<mongo::BSONObj> result;
        if (!helpers::get("queries", "_id", query_id, result, msg)) {
          return false;
        }
        if (result.size() == 0) {
          msg = "Query key is invalid";
          return false;
        }
        mongo::BSONObj query = result[0];
        std::string type = query["type"].str();
        mongo::BSONObj args = query["args"].Obj();

        if (type == "experiment_select") {
          if (!retrieve_experiment_select_query(user_key, query, regions, msg)) {
            return false;
          }
        } else if (type == "intersect") {
          if (!retrieve_intersection_query(user_key, query, regions, msg)) {
            return false;
          }
        } else if (type == "merge") {
          if (!retrieve_merge_query(user_key, query, regions, msg)) {
            return false;
          }
        } else if (type == "annotation_select") {
          if (!retrieve_annotation_select_query(user_key, query, regions, msg)) {
            return false;
          }
        } else if (type == "filter") {
          if (!retrieve_filter_query(user_key, query, regions, msg)) {
            return false;
          }
        } else if (type == "tiling") {
          if (!retrieve_tiling_query(query, regions, msg)) {
            return false;
          }
        } else if (type == "aggregate") {
          if (!process_aggregate(user_key, query, regions, msg)) {
            return false;
          }

        } else {
          msg = "Unknown query type";
          return false;
        }

        return true;
      }

      bool get_experiments_by_query(const std::string &user_key, const std::string &query_id,
                                    std::vector<utils::IdName> &experiments_name, std::string &msg)
      {
        ChromosomeRegionsList chromossome_regions;
        if (!retrieve_query(user_key, query_id, chromossome_regions, msg)) {
          return false;
        }

        std::set<DatasetId> datasets_it_set;
        for (ChromosomeRegionsList::const_iterator it = chromossome_regions.begin(); it != chromossome_regions.end(); it++) {
          Regions regions = it->second;
          for (RegionsIterator it = regions->begin(); it != regions->end(); it++) {
            datasets_it_set.insert(it->dataset_id());
          }
        }

        std::vector<DatasetId> datasets_it(datasets_it_set.size());
        std::copy(datasets_it_set.begin(), datasets_it_set.end(), datasets_it.begin());

        mongo::BSONObjBuilder experiments_query_builder;
        experiments_query_builder << KeyMapper::DATASET() << helpers::build_condition_array<DatasetId>(datasets_it, "$in");

        mongo::BSONObj o = experiments_query_builder.obj();
        std::auto_ptr<mongo::DBClientCursor> cursor;

        mongo::ScopedDbConnection c(config::get_mongodb_server());
        cursor = c->query(helpers::collection_name(Collections::EXPERIMENTS()), o);
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


      bool count_regions(const std::string &user_key, const std::string &query_id,
                         size_t &count, std::string &msg)
      {
        std::vector<mongo::BSONObj> result;
        if (!helpers::get("queries", "_id", query_id, result, msg)) {
          return false;
        }
        if (result.size() == 0) {
          msg = "Query key is invalid";
          return false;
        }
        mongo::BSONObj query = result[0];
        std::string type = query["type"].str();
        mongo::BSONObj args = query["args"].Obj();

        count = 0;
        ChromosomeRegionsList regions;
        if (!retrieve_query(user_key, query_id,  regions, msg)) {
          return false;
        }

        count = 0;
        for (ChromosomeRegionsList::const_iterator it = regions.begin(); it != regions.end(); it++) {
          std::string chromosome = it->first;
          Regions regions = it->second;
          count += regions->size();
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
        if (args.hasField("upload_info.upload_end")) {
          experiments_query_builder.append(args["upload_info.upload_end"]);
        }
        experiments_query_builder.append("upload_info.done", true);
        return experiments_query_builder.obj();
      }

      bool build_experiment_query(const int start, const int end, const std::string &experiment_name,
                                  const std::string &user_key,  mongo::BSONObj &regions_query, std::string &msg)
      {
        mongo::ScopedDbConnection c(config::get_mongodb_server());
        std::string norm_name = utils::normalize_name(experiment_name);
        std::auto_ptr<mongo::DBClientCursor> cursor =
          c->query(helpers::collection_name(Collections::EXPERIMENTS()), BSON("norm_name" << norm_name));

        if (!cursor->more()) {
          msg = "Experiments name '" + experiment_name + "' not found";
          c.done();
          return false;
        }

        mongo::BSONObj p = cursor->next();
        mongo::BSONElement dataset_id = p.getField(KeyMapper::DATASET());
        mongo::BSONObjBuilder regions_query_builder;
        regions_query_builder.append(KeyMapper::DATASET(), dataset_id.Int());
        regions_query_builder.append(KeyMapper::START(), BSON("$lte" << end));
        regions_query_builder.append(KeyMapper::END(), BSON("$gte" << start));
        regions_query = regions_query_builder.obj();

        c.done();

        return true;
      }

      bool build_experiment_query(const std::string &user_key, const mongo::BSONObj &query,
                                  mongo::BSONObj &regions_query, std::string &msg)
      {
        mongo::BSONObj args = query["args"].Obj();

        mongo::BSONArrayBuilder datasets_array_builder;

        if (args["has_filter"].Bool()) {
          const mongo::BSONObj query = build_query(args);
          mongo::ScopedDbConnection c(config::get_mongodb_server());
          std::auto_ptr<mongo::DBClientCursor> cursor = c->query(helpers::collection_name(Collections::EXPERIMENTS()), query);
          while (cursor->more()) {
            mongo::BSONObj p = cursor->next();
            mongo::BSONElement dataset_id = p.getField(KeyMapper::DATASET());
            datasets_array_builder.append(dataset_id.Int());
          }
          c.done();
        }

        mongo::BSONArray datasets_array = datasets_array_builder.arr();

        mongo::BSONObjBuilder regions_query_builder;
        if (args["has_filter"].Bool()) {
          regions_query_builder.append(KeyMapper::DATASET(), BSON("$in" << datasets_array));
        }

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

      bool build_annotation_query(const std::string &user_key, const mongo::BSONObj &query,
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

        mongo::ScopedDbConnection c(config::get_mongodb_server());
        mongo::BSONObj annotation_query = annotations_query_builder.obj();
        std::auto_ptr<mongo::DBClientCursor> cursor = c->query(helpers::collection_name(Collections::ANNOTATIONS()), annotation_query);
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

      bool retrieve_experiment_select_query(const std::string &user_key, const mongo::BSONObj &query,
                                            ChromosomeRegionsList &regions, std::string &msg)
      {

        mongo::BSONObj regions_query;
        if (!build_experiment_query(user_key, query, regions_query, msg)) {
          return false;
        }

        mongo::BSONObj args = query["args"].Obj();

        std::vector<std::string> chromosomes;
        std::vector<mongo::BSONElement> chr_arr = args["chromosomes"].Array();
        std::vector<mongo::BSONElement>::iterator it;
        for (it = chr_arr.begin(); it != chr_arr.end(); ++it) {
          chromosomes.push_back(it->str());
        }

        std::vector<ChromosomeRegionsList> genome_regions;

        // get region data for all genomes
        std::vector<mongo::BSONElement> genome_arr = args["norm_genomes"].Array();
        std::vector<mongo::BSONElement>::iterator git;
        for (git = genome_arr.begin(); git != genome_arr.end(); ++git) {
          ChromosomeRegionsList reg;
          if (!retrieve::get_regions(git->str(), chromosomes, regions_query, reg, msg)) {
            return false;
          }
          genome_regions.push_back(reg);
        }

        // merge region data of all genomes
        std::vector<ChromosomeRegionsList>::iterator rit = genome_regions.begin();
        ChromosomeRegionsList last = *rit;
        rit++;
        for (; rit != genome_regions.end(); ++rit) {
          ChromosomeRegionsList res;
          if (!algorithms::merge_chromosome_regions(last, *rit, res)) {
            return false;
          }
          last = res;
        }
        regions = last;

        return true;
      }


      bool retrieve_annotation_select_query(const std::string &user_key, const mongo::BSONObj &query,
                                            ChromosomeRegionsList &regions, std::string &msg)
      {
        mongo::BSONObj regions_query;
        if (!build_annotation_query(user_key, query, regions_query, msg)) {
          return false;
        }

        mongo::BSONObj args = query["args"].Obj();
        std::vector<std::string> chromosomes;
        std::vector<mongo::BSONElement> chr_arr = args["chromosomes"].Array();
        std::vector<mongo::BSONElement>::iterator it;
        for (it = chr_arr.begin(); it != chr_arr.end(); ++it) {
          chromosomes.push_back(it->str());
        }

        std::vector<ChromosomeRegionsList> genome_regions;

        // get region data for all genomes
        std::vector<mongo::BSONElement> genome_arr = args["norm_genomes"].Array();
        std::vector<mongo::BSONElement>::iterator git;
        for (git = genome_arr.begin(); git != genome_arr.end(); ++git) {
          ChromosomeRegionsList reg;
          if (!retrieve::get_regions(git->str(), chromosomes, regions_query, reg, msg)) {
            return false;
          }
          genome_regions.push_back(reg);
        }

        // merge region data of all genomes
        std::vector<ChromosomeRegionsList>::iterator rit = genome_regions.begin();
        ChromosomeRegionsList last = *rit;
        rit++;
        for (; rit != genome_regions.end(); ++rit) {
          ChromosomeRegionsList res;
          if (!algorithms::merge_chromosome_regions(last, *rit, res)) {
            return false;
          }
          last = res;
        }
        regions = last;

        return true;
      }

      bool retrieve_intersection_query(const std::string &user_key, const mongo::BSONObj &query,
                                       ChromosomeRegionsList &regions, std::string &msg)
      {
        mongo::BSONObj args = query["args"].Obj();

        // load both region sets.
        ChromosomeRegionsList regions_a;
        bool ret = retrieve_query(user_key, args["qid_1"].str(), regions_a, msg);
        if (!ret) {
          msg = "Cannot retrieve first region set: " + msg;
          return false;
        }

        ChromosomeRegionsList regions_b;
        ret = retrieve_query(user_key, args["qid_2"].str(), regions_b, msg);
        if (!ret) {
          msg = "Cannot retrieve second region set: " + msg;
          return false;
        }

        ret = algorithms::intersect(regions_a, regions_b, regions);
        if (!ret) {
          return false;
        }

        return true;
      }


      bool retrieve_merge_query(const std::string &user_key, const mongo::BSONObj &query,
                                ChromosomeRegionsList &regions, std::string &msg)
      {
        mongo::BSONObj args = query["args"].Obj();

        // load both region sets.
        ChromosomeRegionsList regions_a;
        bool ret = retrieve_query(user_key, args["qid_1"].str(), regions_a, msg);
        if (!ret) {
          msg = "Cannot retrieve first region set: " + msg;
          return false;
        }

        ChromosomeRegionsList regions_b;
        ret = retrieve_query(user_key, args["qid_2"].str(), regions_b, msg);
        if (!ret) {
          msg = "Cannot retrieve second region set: " + msg;
          return false;
        }

        if (!algorithms::merge_chromosome_regions(regions_a, regions_b, regions)) {
          return false;
        }

        return true;
      }


      bool filter_region(const Region &region, const std::string &field, const int &pos,
                         Metafield &metafield, const std::string &chrom, FilterBuilder::FilterPtr filter)
      {
        if (field.compare("START") == 0) {
          return filter->is(region.start());
        }
        if (field.compare("END") == 0) {
          return filter->is(region.end());
        }

        // TODO: optimize for "@AGG." values
        if (field[0] == '@') {
          std::string value;
          std::string msg;
          if (!metafield.process(field, chrom, region, value, msg)) {
            EPIDB_LOG_ERR(msg);
            return false;
          }
          return filter->is(value);
        } else {
          return filter->is(region.value(pos));
        }
      }

      bool retrieve_filter_query(const std::string &user_key, const mongo::BSONObj &query,
                                 ChromosomeRegionsList &filtered_regions, std::string &msg)
      {
        mongo::BSONObj args = query["args"].Obj();

        // load original query
        ChromosomeRegionsList regions;
        bool ret = retrieve_query(user_key, args["query"].str(), regions, msg);
        if (!ret) {
          msg = "Cannot retrieve region set: " + msg;
          return false;
        }

        std::string type = args["type"].str();
        std::string operation = args["operation"].str();
        std::string value = args["value"].str();
        std::string field = args["field"].str();

        bool error;
        FilterBuilder::FilterPtr filter;

        if (type.compare("string") == 0) {
          filter = FilterBuilder::getInstance().build(field, operation, value, error, msg);
          if (error) {
            return false;
          }
        } else if (type.compare("number") == 0 || type.compare("integer") == 0 || type.compare("double") == 0) {
          filter = FilterBuilder::getInstance().build(field, operation, atof(value.c_str()), error, msg);
          if (error) {
            return false;
          }
        } else {
          msg = "Invalid type. Valid types are: string, number, integer, double.";
          return false;
        }

        std::string err;
        DatasetId dataset_id = -1;
        int pos;
        datatypes::COLUMN_TYPES column_type;

        size_t total = 0;
        size_t removed = 0;
        size_t keep = 0;

        Metafield metafield;
        for (ChromosomeRegionsList::const_iterator it = regions.begin(); it != regions.end(); it++) {
          std::string chromosome = it->first;
          Regions regs = it->second;
          Regions saved = build_regions();
          for (RegionsIterator it = regs->begin(); it != regs->end(); it++) {
            const Region &region = *it;
            if (!dba::Metafield::is_meta(field)) {
              if (region.dataset_id() != dataset_id) {
                dataset_id = region.dataset_id();
                if (!dba::experiments::get_field_pos(dataset_id, field, pos, column_type, msg)) {
                  return false;
                }
              }
              dataset_id = region.dataset_id();
            }
            total++;

            // TODO: use column_type for better filtering. i.e. type conversion
            if (filter_region(region, field, pos, metafield, chromosome, filter)) {
              saved->push_back(region);
              keep++;
            } else {
              removed++;
            }
          }
          ChromosomeRegions chr_region(chromosome, saved);
          filtered_regions.push_back(chr_region);
        }

        return true;
      }

      bool add_tiling(const std::string &genome, const size_t &tiling_size,
                      DatasetId &dataset_id, std::string &msg)
      {

        mongo::ScopedDbConnection c(config::get_mongodb_server());

        std::string norm_genome = utils::normalize_name(genome);

        std::auto_ptr<mongo::DBClientCursor> data_cursor =
          c->query(helpers::collection_name(Collections::TILINGS()),
                   QUERY("norm_genome" << norm_genome << "tiling_size" << (int) tiling_size), 0, 1);

        if (!c->getLastError().empty()) {
          msg = c->getLastError();
          c.done();
          return false;
        }
        if (data_cursor->more()) {
          mongo::BSONObj result = data_cursor->next().getOwned();
          dataset_id = result.getField(KeyMapper::DATASET()).Int();
          c.done();
          return true;
        }

        int t_id;
        if (!helpers::get_counter("tiling", t_id, msg))  {
          c.done();
          return false;
        }
        std::string tiling_id = "tr" + utils::integer_to_string(t_id);

        if (!helpers::get_counter("datasets", dataset_id, msg))  {
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


      bool retrieve_tiling_query(const mongo::BSONObj &query,
                                 ChromosomeRegionsList &regions, std::string &msg)
      {
        mongo::BSONObj args = query["args"].Obj();

        const std::string genome = args["genome"].str();
        const std::string norm_genome = args["norm_genome"].str();
        const size_t tiling_size = args["size"].Long();

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

        // generate tiling regions for all chromosomes
        ChromosomeRegionsList chromosome_regions_list;

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
          Regions regs = build_regions();
          for (size_t i = 0; i + tiling_size < chromosome_info.size; i += tiling_size) {
            regs->push_back(Region(i, i + tiling_size, tiling_id));
          }
          regions.push_back(ChromosomeRegions(*cit, regs));
        }

        return true;
      }

      bool process_aggregate(const std::string &user_key, const mongo::BSONObj &query,
                             ChromosomeRegionsList &regions, std::string &msg)
      {
        mongo::BSONObj args = query["args"].Obj();
        const std::string query_id = args["data_id"].str();
        const std::string regions_id = args["ranges_id"].str();
        const std::string field = args["field"].str();

        ChromosomeRegionsList data;
        bool ret = retrieve_query(user_key, query_id, data, msg);
        if (!ret) {
          msg = "Cannot retrieve region set: " + msg;
          return false;
        }

        ChromosomeRegionsList ranges;
        ret = retrieve_query(user_key, regions_id, ranges, msg);
        if (!ret) {
          msg = "Cannot retrieve region set: " + msg;
          return false;
        }

        return algorithms::aggregate(data, ranges, field, regions, msg);
      }

      bool get_columns_from_dataset(const DatasetId &dataset_id, std::vector<mongo::BSONObj> &columns, std::string &msg)
      {
        if (dataset_id == DATASET_EMPTY_ID) {
          return true;
        }

        mongo::BSONObjBuilder experiments_query_builder;
        experiments_query_builder << KeyMapper::DATASET() << dataset_id;

        mongo::BSONObj o = experiments_query_builder.obj();
        std::auto_ptr<mongo::DBClientCursor> cursor;

        mongo::ScopedDbConnection c(config::get_mongodb_server());
        cursor = c->query(helpers::collection_name(Collections::EXPERIMENTS()), o);

        while (cursor->more()) {
          mongo::BSONObj experiment = cursor->next();
          if (experiment.hasField("columns")) {
            int s_count = 0;
            int n_count = 0;

            std::vector<mongo::BSONElement> tmp_columns = experiment["columns"].Array();
            BOOST_FOREACH(const mongo::BSONElement & e, tmp_columns) {
              mongo::BSONObj column = e.Obj();
              const std::string &column_type = column["column_type"].str();
              const std::string &column_name = column["name"].str();

              int pos = -1;

              mongo::BSONObjBuilder bob;
              bob.appendElements(column);

              if (column_name != "CHROMOSOME" && column_name != "START" &&  column_name != "END") {
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
            c.done();
            return true;
          } else {
            c.done();
            return false;
          }
        }

        cursor = c->query(helpers::collection_name(Collections::ANNOTATIONS()), o);
        while (cursor->more()) {
          mongo::BSONObj annotation = cursor->next().getOwned();
          if (annotation.hasField("columns")) {
            int s_count = 0;
            int n_count = 0;
            std::vector<mongo::BSONElement> tmp_columns = annotation["columns"].Array();
            BOOST_FOREACH(const mongo::BSONElement & e, tmp_columns) {
              mongo::BSONObj column = e.Obj();
              const std::string &column_type = column["column_type"].str();
              const std::string &column_name = column["name"].str();

              int pos = -1;

              mongo::BSONObjBuilder bob;
              bob.appendElements(column);

              if (column_name != "CHROMOSOME" && column_name != "START" &&  column_name != "END") {
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
            c.done();
            return true;
          } else {
            std::cerr <<  "Annotation dataset" << dataset_id << " does not have columns!!" << std::endl;
            msg = "blah 2";
            c.done();
            return false;
          }
        }

        cursor = c->query(helpers::collection_name(Collections::TILINGS()), o);
        while (cursor->more()) {
          mongo::BSONObj tiling = cursor->next().getOwned();
          if (tiling.hasField("columns")) {
            std::vector<mongo::BSONElement> tmp_columns = tiling["columns"].Array();
            BOOST_FOREACH(const mongo::BSONElement & e, tmp_columns) {
              columns.push_back(e.Obj().getOwned());
            }
            c.done();
            return true;
          } else {
            std::cerr << "Tiling dataset" << dataset_id << " does not have columns!!" << std::endl;
            msg = "blah ";
            c.done();
            return false;
          }
        }
        c.done();
        msg = Error::m(ERR_DATASET_NOT_FOUND, dataset_id);
        return false;
      }
    }
  }
}
