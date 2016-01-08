//
//  score_matrix.cpp
//  DeepBlue Epigenomic Data Server

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
//  Created by Felipe Albrecht on 03.02.15.
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

#include <future>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include <thread>
#include <tuple>

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

    //         Okay,    msg,   experiment name, chromosome,  regions
    std::tuple<bool, std::string, std::string, std::string, std::shared_ptr<std::vector<algorithms::Accumulator>>>
    summarize_experiment(const std::string& norm_genome,
                         const std::pair<std::string, dba::columns::ColumnTypePtr>& experiment_format,
                         const ChromosomeRegions& chromosome,
                         processing::StatusPtr status)
    {
      std::string msg;
      std::shared_ptr<std::vector<algorithms::Accumulator>> regions_accs = std::make_shared<std::vector<algorithms::Accumulator>>();

      // Check if processing was canceled
      bool is_canceled = false;
      if (!status->is_canceled(is_canceled, msg)) {
        return std::make_tuple(true, msg, "", "", regions_accs);
      }
      if (is_canceled) {
        msg = Error::m(ERR_REQUEST_CANCELED);
        return std::make_tuple(false, msg, "", "", regions_accs);
      }

      const int BLOCK_SIZE = 100;
      size_t region_pos = 0;
      while (region_pos < chromosome.second.size()) {

        Regions ranges;
        for (size_t i = 0; i < BLOCK_SIZE && region_pos < chromosome.second.size(); i++, region_pos++) {
          ranges.emplace_back(chromosome.second[region_pos]->clone());
        }

        mongo::BSONObj regions_query;
        if (!dba::query::build_experiment_query(ranges[0]->start(), ranges[ranges.size() - 1]->end(), experiment_format.first, regions_query, msg)) {
          return std::make_tuple(false, msg, "", "", regions_accs);
        }

        Regions data;
        if (!dba::retrieve::get_regions(norm_genome, chromosome.first, regions_query, false, status, data, msg)) {
          return std::make_tuple(false, msg, "", "", regions_accs);
        }

        auto it_ranges = ranges.begin();
        auto it_data_begin = data.begin();

        while (it_ranges != ranges.end()) {
          algorithms::Accumulator acc;

          while ( it_data_begin != data.end()
                  && (*it_data_begin)->end() < (*it_ranges)->start() )  {
            it_data_begin++;
          }

          auto it_data = it_data_begin;
          while (it_data != data.end() &&
                 (*it_ranges)->end() >= (*it_data)->start() ) {

            if (((*it_ranges)->start() <= (*it_data)->end()) && ((*it_ranges)->end() >= (*it_data)->start())) {
              acc.push((*it_data)->value(experiment_format.second->pos()));
            }
            it_data++;
          }
          regions_accs->push_back(acc);
          it_ranges++;
        }
      }
      return std::make_tuple(true, "", experiment_format.first, chromosome.first, regions_accs);
    }

    bool score_matrix(const std::vector<std::pair<std::string, std::string>> &experiments_formats, const std::string & aggregation_function, const std::string & regions_query_id, const std::string & user_key, processing::StatusPtr status, std::string & matrix, std::string & msg)
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

      std::map<std::string, std::map<std::string, std::shared_ptr<std::vector<algorithms::Accumulator>>>> chromosome_accs;

      std::vector<std::future<std::tuple<bool, std::string, std::string, std::string, std::shared_ptr<std::vector<algorithms::Accumulator>>>>> threads;
      for (auto &experiment_format : norm_experiments_formats) {
        for (auto &chromosome : range_regions) {
          auto t = std::async(std::launch::async, &summarize_experiment,
                              std::ref(norm_genome), std::ref(experiment_format), std::ref(chromosome), status);
          threads.push_back(std::move(t));
        }
      }

      size_t thread_count = threads.size();
      for (size_t i = 0; i < thread_count; ++i) {
        threads[i].wait();
        const auto& result = threads[i].get();
        bool okay = std::get<0>(result);
        if (!okay) {
          msg = std::get<1>(result);
          return false;
        }
        chromosome_accs[std::get<2>(result)][std::get<3>(result)] = std::get<4>(result);
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

      for (auto &chromosome : range_regions) {
        size_t pos = 0;
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
            auto *acc = &chromosome_accs[experiment_format.first][chromosome_name]->at(pos);
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