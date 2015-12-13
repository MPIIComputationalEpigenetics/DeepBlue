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

    bool score_matrix(const std::vector<std::pair<std::string, std::string>> &experiments_formats, const std::string& aggregation_function, const std::string &regions_query_id, const std::string &user_key, processing::StatusPtr status, std::string &matrix, std::string &msg)
    {

      ChromosomeRegionsList range_regions;
      if (!dba::query::retrieve_query(user_key, regions_query_id, status, range_regions, msg)) {
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
          msg = "The column " + experiment_input.second + " (" + datatypes::column_type_to_name(column->type()) + ") in the experiment " + experiment_name + " does not contain numerical values.";
          return false;
        }

        std::pair<std::string, dba::columns::ColumnTypePtr> p(norm_name, column);
        norm_experiments_formats.push_back(p);

        // TODO: check if are all from the same genome
        norm_genome = experiment["norm_genome"].String();
      }

      std::map<std::string, std::vector<algorithms::Accumulator>> chromosome_accs;

      for (auto &experiment_format : norm_experiments_formats) {

        std::vector<algorithms::Accumulator> regions_accs;
        for (auto &chromosome : range_regions) {

          for (auto &region : chromosome.second) {

            // Check if processing was canceled
            bool is_canceled = false;
            if (!status->is_canceled(is_canceled, msg)) {
              return true;
            }
            if (is_canceled) {
              msg = Error::m(ERR_REQUEST_CANCELED);
              return false;
            }

            // ***
            mongo::BSONObj regions_query;
            if (!dba::query::build_experiment_query(region->start(), region->end(), experiment_format.first, regions_query, msg)) {
              return false;
            }

            Regions regions;
            if (!dba::retrieve::get_regions(norm_genome, chromosome.first, regions_query, true, status, regions, msg)) {
              return false;
            }

            algorithms::Accumulator acc;
            for (auto &experiment_region : regions) {
              acc.push(experiment_region->value(experiment_format.second->pos()));
            }
            regions_accs.push_back(acc);
          }
        }
        chromosome_accs[experiment_format.first] = regions_accs;
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
      size_t pos = 0;
      for (auto &chromosome : range_regions) {
        const auto &chromosome_name = chromosome.first;
        const auto &regions = chromosome.second;

        for (const auto& region : regions) {
          // Check if processing was canceled
          bool is_canceled = false;
          if (!status->is_canceled(is_canceled, msg)) {
            return true;
          }
          if (is_canceled) {
            msg = Error::m(ERR_REQUEST_CANCELED);
            return false;
          }

          // ***
          ss << chromosome_name << "\t";
          ss << region->start() << "\t";
          ss << region->end() << "\t";

          bool first = true;
          for (auto &experiment_format : norm_experiments_formats) {
            auto *acc = &chromosome_accs[experiment_format.first][pos];

            if (!first) {
              ss << "\t";
            } else {
              first = false;
            }

            std::string value = acc->string("|");
            if (!value.empty()) {
              if (data_ptr) {
                ss << (*acc.*data_ptr)();
              } else {
                ss << acc->string("|");
              }
            }
          }
          ss << std::endl;
          pos++;
        }
      }

      matrix = ss.str();
      return true;
    }
  }
}