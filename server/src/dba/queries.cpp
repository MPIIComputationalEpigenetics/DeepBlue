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
#include <boost/lexical_cast.hpp>

#include <mongo/bson/bson.h>
#include <mongo/client/dbclient.h>

#include "../regions.hpp"
#include "../log.hpp"
#include "../algorithms/aggregate.hpp"
#include "../algorithms/intersection.hpp"
#include "../algorithms/merge.hpp"
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
        query_id = "q" + boost::lexical_cast<std::string>(query_counter);
        time_t time_;
        time(&time_);

        mongo::BSONObjBuilder stored_query_builder;

        stored_query_builder.append("_id", query_id);
        std::string user_name;
        if (!get_user_name(user_key, user_name, msg)) {
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

        std::set<std::string> experiments_id_set;
        for (ChromosomeRegionsList::const_iterator it = chromossome_regions.begin(); it != chromossome_regions.end(); it++) {
          Regions regions = it->second;
          for (RegionsIterator it = regions->begin(); it != regions->end(); it++) {
            experiments_id_set.insert(*(it->collection_id()));
          }
        }

        std::vector<std::string> experiments_id(experiments_id_set.size());
        std::copy(experiments_id_set.begin(), experiments_id_set.end(), experiments_id.begin());

        mongo::BSONObjBuilder experiments_query_builder;
        experiments_query_builder << "_id" << helpers::build_condition_array(experiments_id, "$in");

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
        if (args.hasField("upload_end")) {
          experiments_query_builder.append(args["upload_end"]);
        }
        experiments_query_builder.append("done", true);
        return experiments_query_builder.obj();
      }

      bool build_experiment_query(const std::string &user_key, const mongo::BSONObj &query,
                                  mongo::BSONObj &regions_query, std::vector<std::string> &experiments_id,
                                  std::string &msg)
      {
        mongo::BSONObj args = query["args"].Obj();

        if (args["has_filter"].Bool()) {
          const mongo::BSONObj query = build_query(args);
          mongo::ScopedDbConnection c(config::get_mongodb_server());
          std::auto_ptr<mongo::DBClientCursor> cursor = c->query(helpers::collection_name(Collections::EXPERIMENTS()), query);
          while (cursor->more()) {
            mongo::BSONObj p = cursor->next();
            mongo::BSONElement experiment_id = p.getField("_id");
            experiments_id.push_back(experiment_id.str());
          }
          c.done();
        }

        mongo::BSONObjBuilder regions_query_builder;
        if (args.hasField("start") && args.hasField("end")) {
          regions_query_builder.append(KeyMapper::START(), BSON("$gte" << args["start"].Long() << "$lte" << args["end"].Long()));
          regions_query_builder.append(KeyMapper::END(), BSON("$gte" << args["start"].Long() << "$lte" << args["end"].Long()));
        } else if (args.hasField("start")) {
          regions_query_builder.append(KeyMapper::START(), BSON("$gte" << args["start"].Long()));
        } else if (args.hasField("end")) {
          regions_query_builder.append(KeyMapper::END(), BSON("$lte" << args["end"].Long()));
        }

        regions_query = regions_query_builder.obj();

        return true;
      }

      bool build_annotation_query(const std::string &user_key, const mongo::BSONObj &query,
                                  mongo::BSONObj &regions_query, std::vector<std::string> &annotations_id,
                                  std::string &msg)
      {
        mongo::BSONObjBuilder annotations_query_builder;
        mongo::BSONObj args = query["args"].Obj();

        // TODO: do the same as build_experiment_query, not verifying the type()
        if (args["norm_annotation"].type() == mongo::Array) {
          annotations_query_builder.append("norm_name", BSON("$in" << args["norm_annotation"]));
        } else {
          annotations_query_builder.append("norm_name", args["norm_annotation"].str());
        }
        annotations_query_builder.append("done", true);

        mongo::ScopedDbConnection c(config::get_mongodb_server());
        mongo::BSONObj annotation_query = annotations_query_builder.obj();
        std::auto_ptr<mongo::DBClientCursor> cursor = c->query(helpers::collection_name(Collections::ANNOTATIONS()), annotation_query);
        while (cursor->more()) {
          mongo::BSONObj p = cursor->next();
          mongo::BSONElement annotation_id = p.getField("_id");
          annotations_id.push_back(annotation_id.str());
        }
        c.done();

        mongo::BSONObjBuilder regions_query_builder;
        if (args.hasField("start") && args.hasField("end")) {
          regions_query_builder.append(KeyMapper::START(), BSON("$lte" << args["end"].Long()));
          regions_query_builder.append(KeyMapper::END(), BSON("$gte" << args["start"].Long()));
        } else if (args.hasField("start")) {
          regions_query_builder.append(KeyMapper::END(), BSON("$gte" << args["start"].Long()));
        } else if (args.hasField("end")) {
          regions_query_builder.append(KeyMapper::START(), BSON("$lte" << args["end"].Long()));
        }

        regions_query = regions_query_builder.obj();

        return true;
      }

      bool retrieve_experiment_select_query(const std::string &user_key, const mongo::BSONObj &query,
                                            ChromosomeRegionsList &regions, std::string &msg)
      {

        mongo::BSONObj regions_query;
        std::vector<std::string> experiments_id;
        if (!build_experiment_query(user_key, query, regions_query, experiments_id, msg)) {
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
          if (!retrieve::get_regions(git->str(), experiments_id, chromosomes,
                                     regions_query, reg, msg)) {
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
        std::vector<std::string> annotations_id;
        if (!build_annotation_query(user_key, query, regions_query, annotations_id, msg)) {
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
          if (!retrieve::get_regions(git->str(), annotations_id, chromosomes,
                                     regions_query, reg, msg)) {
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


      bool filter_region(Region &region, const std::string &field, const std::string &field_key,
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
          return filter->is(region.get(field_key));
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
        std::string field_key;
        if (!KeyMapper::to_short(field, field_key, err)) {
          msg = "Field " + field + " is not being used. (" + err + ")";
          return false;
        }

        size_t total = 0;
        size_t removed = 0;
        size_t keep = 0;

        Metafield metafield;
        for (ChromosomeRegionsList::const_iterator it = regions.begin(); it != regions.end(); it++) {
          std::string chromosome = it->first;
          Regions regs = it->second;
          Regions saved = build_regions();
          for (RegionsIterator it = regs->begin(); it != regs->end(); it++) {
            total++;
            if (filter_region(*it, field, field_key, metafield, chromosome, filter)) {
              saved->push_back(*it);
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
                      std::string &tiling_id, std::string &msg)
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
          tiling_id = result.getField("_id").str();
          c.done();
          return true;
        }

        {
          int t_id;
          if (!helpers::get_counter("tiling", t_id, msg))  {
            c.done();
            return false;
          }
          tiling_id = "t" + boost::lexical_cast<std::string>(t_id);
        }

        mongo::BSONObjBuilder tiling_data_builder;

        std::stringstream name;
        name << "Tiling regions of " << tiling_size << " (Genome " << genome << ")";
        tiling_data_builder.append("_id", tiling_id);
        tiling_data_builder.append("name", name.str());
        tiling_data_builder.append("genome", genome);
        tiling_data_builder.append("norm_genome", norm_genome);
        tiling_data_builder.append("tiling_size", (int) tiling_size);

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

        std::string tiling_id;
        if (!add_tiling(genome, tiling_size, tiling_id, msg)) {
          return false;
        }

        CollectionId id = build_collection_id(tiling_id);

        for (cit = chromosomes.begin(); cit != chromosomes.end(); ++cit) {
          if (!genome_info->get_chromosome(*cit, chromosome_info, msg)) {
            msg = "Chromosome " + *cit + " does not exist on genome " + genome + ".";
            return false;
          }
          Regions regs = build_regions();
          for (size_t i = 0; i + tiling_size < chromosome_info.size; i += tiling_size) {
            regs->push_back(Region(i, i + tiling_size, id));
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

        return algorithms::aggregate(data, ranges, field, regions);
      }
    }
  }
}
