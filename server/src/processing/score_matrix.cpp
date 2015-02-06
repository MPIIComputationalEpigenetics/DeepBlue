//
//  score_matrix.cpp
//  epidb
//
//  Created by Felipe Albrecht on 03.02.15.
//  Copyright (c) 2015 Max Planck Institute for Computer Science. All rights reserved.
//

#include <map>
#include <sstream>
#include <vector>

#include "../algorithms/accumulator.hpp"

#include "../datatypes/regions.hpp"

#include "../dba/column_types.hpp"
#include "../dba/experiments.hpp"
#include "../dba/helpers.hpp"
#include "../dba/queries.hpp"

#include "../engine/commands.hpp"
#include "../extras/serialize.hpp"

#include "../errors.hpp"
#include "../log.hpp"

namespace epidb {
  namespace processing {

    bool score_matrix(const std::vector<std::pair<std::string, std::string>> &experiments_formats, const std::string& aggregation_function, const std::string &regions_query_id, const std::string &user_key, std::string &matrix, std::string &msg)
    {

      ChromosomeRegionsList range_regions;
      if (!dba::query::retrieve_query(user_key, regions_query_id, range_regions, msg)) {
        return false;
      }

      algorithms::GetDataPtr data_ptr = algorithms::get_function_data(aggregation_function);
      if (!data_ptr && aggregation_function != "acc") {
        msg = "Aggregation function " + aggregation_function + " is invalid.";
        return false;
      }

      /// Experiments, Chromosomes, Values
      std::string norm_genome;

      // Change names to norm_names and columns
      std::vector<std::pair<std::string, dba::columns::ColumnTypePtr>> norm_experiments_formats;
      for (auto &experiment_input : experiments_formats) {
        mongo::BSONObj experiment;
        const std::string &experiment_name = experiment_input.first;
        if (!dba::experiments::by_name(experiment_name, experiment, msg)) {
          return false;
        }
        std::string norm_name = experiment["norm_name"].String();
        DatasetId dataset_id = experiment[dba::KeyMapper::DATASET()].Int();
        dba::columns::ColumnTypePtr column;

        if (!dba::experiments::get_field_pos(dataset_id, experiment_input.second, column, msg)) {
          return false;
        }

        if (column->type() != datatypes::COLUMN_INTEGER && column->type() != datatypes::COLUMN_DOUBLE && column->type() == datatypes::COLUMN_RANGE) {
          std::cerr << column->type() << std::endl;
          msg = "The column " + experiment_input.second + " (" + datatypes::column_type_to_name(column->type()) + ") in the experiment " + experiment_name + " does not contain numerical values.";
          return false;
        }

        std::pair<std::string, dba::columns::ColumnTypePtr> p(norm_name, column);
        norm_experiments_formats.push_back(p);

        // TODO: check if are all from the same genome
        norm_genome = experiment["norm_genome"].String();
      }

      std::map<std::string, std::vector<std::pair<RegionPtr, std::vector<algorithms::Accumulator>>>> chromosome_accs;
      for (auto &chromosome : range_regions) {
        // Get the accumulators from each region
        std::vector<std::pair<RegionPtr, std::vector<algorithms::Accumulator>>> regions_accs;
        for (auto &region : chromosome.second) {
          // Get the accumulator from each experiment
          std::vector<algorithms::Accumulator> experiments_accs;
          for (auto &experiment_format : norm_experiments_formats) {
            mongo::BSONObj regions_query;
            if (!dba::query::build_experiment_query(region->start(), region->end(), experiment_format.first, user_key, regions_query, msg)) {
              return false;
            }

            Regions regions;
            if (!dba::retrieve::get_regions(norm_genome, chromosome.first, regions_query, regions, msg)) {
              return false;
            }
            algorithms::Accumulator acc;
            for (auto &experiment_region : regions) {
              acc.push(experiment_region->value(experiment_format.second->pos()));
            }
            experiments_accs.push_back(acc);
          }
          std::pair<RegionPtr, std::vector<algorithms::Accumulator> > region_accs(std::move(region), experiments_accs);
          regions_accs.push_back(std::move(region_accs));
        }
        chromosome_accs[chromosome.first] = std::move(regions_accs);
      }


      std::stringstream ss;
      ss << "CHROMOSOME\t";
      ss << "START\t";
      ss << "END\t";
      bool first = true;
      for (auto &experiments_format : experiments_formats) {
        if (!first) {
          ss << "\t";
        }
        ss << experiments_format.first;
        first = false;
      }
      ss << std::endl;
      for (auto chromosome_accs_it = chromosome_accs.begin(); chromosome_accs_it != chromosome_accs.end(); chromosome_accs_it++ ) {
        const std::string &chromosome = chromosome_accs_it->first;
        auto &region_accs = chromosome_accs_it->second;
        for (auto regions_accs_it = region_accs.begin(); regions_accs_it != region_accs.end(); regions_accs_it++) {

          RegionPtr region = std::move(regions_accs_it->first);
          std::vector<algorithms::Accumulator> &experiments_accs = regions_accs_it->second;

          ss << chromosome << "\t";
          ss << region->start() << "\t";
          ss << region->end() << "\t";

          bool first = true;
          for (std::vector<algorithms::Accumulator>::iterator accs_it = experiments_accs.begin();
               accs_it != experiments_accs.end(); accs_it++) {

            if (!first) {
              ss << "\t";
            } else {
              first = false;
            }

            std::string value = accs_it->string("|");
            if (!value.empty()) {
              if (data_ptr) {
                ss << ((*accs_it).*data_ptr)();
              } else {
                ss << accs_it->string("|");
              }
            }
          }
          ss << std::endl;
        }
      }

      matrix = ss.str();

      return true;
    }
  }
}