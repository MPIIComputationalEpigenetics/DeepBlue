//
//  binning.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 29.12.14.
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

#include <iterator>
#include <string>

#include "../cache/column_dataset_cache.hpp"

#include "../dba/queries.hpp"

namespace epidb {
  namespace processing {
    bool binning(const std::string& query_id, const std::string& column_name, const int bars, const std::string& user_key, processing::StatusPtr status, mongo::BSONObj& result, std::string& msg) {


      if (bars <= 0) {
        msg = "There must be at least one bar in the binning";
        return false;
      }

      if (bars >= 65536) {
        msg = "There must be at no more than 65536 bars";
        return false;
      }

      ChromosomeRegionsList chromosomeRegionsList;
      if (!dba::query::retrieve_query(user_key, query_id, status, chromosomeRegionsList, msg)) {
        return false;
      }

      size_t total_size = 0;
      for (const auto& chromosomeRegions: chromosomeRegionsList) {
        total_size += chromosomeRegions.second.size();
      }

      Regions all_regions(total_size);

      for (const auto& chromosomeRegions: chromosomeRegionsList) {
        Regions to_move = std::move(chromosomeRegions.second);
        all_regions.insert(all_regions.end(),
          std::make_move_iterator(to_move.begin()), std::make_move_iterator(to_move.end())
        );
      }

      if (all_regions.empty() ) {
        return true;
      }

      std::sort(all_regions.begin(), all_regions.end(),
        [column_name](const RegionPtr & a, const RegionPtr & b) -> bool
          {
            std::string msg;
            int pos_a;
            int pos_b;
            if (!cache::get_column_position_from_dataset(a->dataset_id(), column_name, pos_a, msg)) {
              return false;
            }
            if (!cache::get_column_position_from_dataset(b->dataset_id(), column_name, pos_b, msg)) {
              return false;
            }
            return a->value(pos_a) <  b->value(pos_b);
          }
      );

      int min_column_pos;
      cache::get_column_position_from_dataset(all_regions.front()->dataset_id(), column_name, min_column_pos, msg);
      Score min = all_regions.front()->value(min_column_pos);

      int max_column_pos;
      cache::get_column_position_from_dataset(all_regions.back()->dataset_id(), column_name, max_column_pos, msg);
      Score max = all_regions.back()->value(max_column_pos);

      Score step = (max - min) / bars;

      std::vector<long> bar_counts(bars);
      int actual_bar = 0;
      int actual_count = 0;


      for (const RegionPtr& region: all_regions) {
        int column_pos;
        if (!cache::get_column_position_from_dataset(region->dataset_id(), column_name, column_pos, msg)) {
          return false;
        }
        Score value = region->value(column_pos);

        if (value <= (min + (step  * (actual_bar + 1)))) {
          actual_count++;
        } else {
          bar_counts[actual_bar] = actual_count;

          actual_bar = static_cast<int>(floor((value - min) / step));

          actual_count = 1;
        }
      }
      bar_counts[actual_bar] = actual_count;

      std::vector<Score> scores;
      for (int i = 0; i <= bars; i++) {
        scores.push_back((min + (step * i)));
      }

      mongo::BSONObjBuilder bob;
      bob.append("ranges", utils::build_array(scores));
      bob.append("counts", utils::build_array(bar_counts));

      result = bob.obj();

      return true;
    }
  }
}