//
//  score_matrix.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 03.02.15.
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

#include "../cache/column_dataset_cache.hpp"

#include "../datatypes/regions.hpp"
#include "../datatypes/user.hpp"

#include "../dba/column_types.hpp"
#include "../dba/experiments.hpp"
#include "../dba/helpers.hpp"
#include "../dba/queries.hpp"
#include "../dba/retrieve.hpp"

#include "../engine/commands.hpp"

#include "../extras/serialize.hpp"

#include "../threading/semaphore.hpp"

#include "../errors.hpp"
#include "../log.hpp"
#include "processing.hpp"

namespace epidb {
  namespace processing {

    //         Okay,    msg,   experiment name, chromosome,  regions
    std::tuple<bool, std::string, std::string, std::string, std::shared_ptr<std::vector<std::string>>>
    summarize_experiment(const std::string& aggregation_function, const std::string& norm_genome,
                         const std::pair<std::string, dba::columns::ColumnTypePtr>& experiment_format,
                         const ChromosomeRegions& chromosome, threading::SemaphorePtr sem,
                         processing::StatusPtr status)
    {
      status->start_operation(PROCESS_SCORE_MATRIX);

      std::string msg;
      std::shared_ptr<std::vector<std::string>> regions_accs = std::make_shared<std::vector<std::string>>();

      algorithms::GetDataPtr data_ptr = algorithms::get_function_data(aggregation_function);
      if (!data_ptr && aggregation_function != "acc") {
        msg = "Aggregation function " + aggregation_function + " is invalid.";
        return std::make_tuple(false, msg, "", "", regions_accs);
      }

      const int BLOCK_SIZE = 100;
      size_t region_pos = 0;
      while (region_pos < chromosome.second.size()) {

        // Check if processing was canceled
        bool is_canceled = false;
        if (!status->is_canceled(is_canceled, msg)) {
          sem->up();
          return std::make_tuple(false, msg, "", "", regions_accs);
        }
        if (is_canceled) {
          msg = Error::m(ERR_REQUEST_CANCELED);
          sem->up();
          return std::make_tuple(false, msg, "", "", regions_accs);
        }
        ////////////////////////////////////////////////////////////

        Regions ranges;
        for (size_t i = 0; i < BLOCK_SIZE && region_pos < chromosome.second.size(); i++, region_pos++) {
          ranges.emplace_back(chromosome.second[region_pos]->clone());
        }

        mongo::BSONObj regions_query;
        if (!dba::query::build_experiment_query(ranges[0]->start(), ranges[ranges.size() - 1]->end(),
                                                utils::normalize_name(experiment_format.first), regions_query, msg)) {
          sem->up();
          return std::make_tuple(false, msg, "", "", regions_accs);
        }

        Regions data;
        if (!dba::retrieve::get_regions(norm_genome, chromosome.first, regions_query, false, status, data, msg)) {
          sem->up();
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

            auto begin = std::max((*it_data)->start(), (*it_ranges)->start());
            auto end =  std::min((*it_data)->end(), (*it_ranges)->end());

            double overlap_length = end - begin;
            double original_length = (*it_data)->end() - (*it_data)->start();

            auto correct_offset = (overlap_length / original_length );


            if (((*it_ranges)->start() <= (*it_data)->end()) && ((*it_ranges)->end() >= (*it_data)->start())) {
              acc.push((*it_data)->value(experiment_format.second->pos()) * correct_offset);
            }
            it_data++;
          }

          std::string value;
          if (acc.count()) {
            if (data_ptr) {
              value = utils::score_to_string( (acc.*data_ptr)() );
            } else {
              value = acc.string("|");
            }
          }

          if (!status->sum_and_check_size(value.length() * sizeof(char))) {
            msg = "Memory exhausted. Used "  + utils::size_t_to_string(status->total_size()) + "bytes of " + utils::size_t_to_string(status->maximum_size()) + "bytes allowed. Please, select a smaller initial dataset, for example, selecting fewer chromosomes)"; // TODO: put a better error msg.
            return std::make_tuple(false, msg, "", "", regions_accs);
          }

          std::cerr << value << std::endl;

          regions_accs->push_back(value);
          it_ranges++;
        }

        // Remove the size of the region, keeping only the size of the Stored score.
        for (const auto& r : data) {
          status->subtract_size(r->size());
        }
      }

      sem->up();
      return std::make_tuple(true, "", experiment_format.first, chromosome.first, regions_accs);
    }

    bool score_matrix(const datatypes::User& user,
                      const std::vector<std::pair<std::string, std::string>> &experiments_formats,
                      const std::string & aggregation_function, const std::string & regions_query_id,
                      processing::StatusPtr status, StringBuilder &sb, std::string & msg)
    {

      ChromosomeRegionsList range_regions;
      if (!dba::query::retrieve_query(user, regions_query_id, status, range_regions, msg)) {
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

        if (!cache::get_column_type_from_dataset(dataset_id, experiment_input.second, column, msg)) {
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

      std::map<std::string, std::map<std::string, std::shared_ptr<std::vector<std::string>>>> chromosome_accs;

      // We only allow 32 simultaneous threads
      threading::SemaphorePtr sem = threading::build_semaphore(32);

      std::vector<std::future<std::tuple<bool, std::string, std::string, std::string, std::shared_ptr<std::vector<std::string>>>>> threads;
      for (auto &experiment_format : norm_experiments_formats) {
        for (auto &chromosome : range_regions) {
          sem->down();
          auto t = std::async(std::launch::async, &summarize_experiment,
                              std::ref(aggregation_function), std::ref(norm_genome),
                              std::ref(experiment_format), std::ref(chromosome),
                              sem, status);
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

      sb.append("CHROMOSOME\t");
      sb.append("START\t");
      sb.append("END\t");

      bool first = true;
      for (auto &experiments_format : experiments_formats) {
        if (!first) {
          sb.tab();
        }
        sb.append(experiments_format.first);
        first = false;
      }
      sb.endLine();

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
          sb.append(chromosome_name);
          sb.tab();
          sb.append(utils::integer_to_string(region->start()));
          sb.tab();
          sb.append(utils::integer_to_string(region->end()));
          sb.tab();

          bool first = true;
          for (auto &experiment_format : norm_experiments_formats) {
            auto *acc = &chromosome_accs[experiment_format.first][chromosome_name]->at(pos);
            if (!first) {
              sb.tab();
            } else {
              first = false;
            }
            sb.append(*acc);
          }
          sb.endLine();
          pos++;
        }
      }

      return true;
    }
  }
}