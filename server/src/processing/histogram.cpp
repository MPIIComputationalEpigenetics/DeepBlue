//
//  histogram.cpp
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
    bool histogram(const std::string& query_id, const std::string& column_name, const int bars, const std::string& user_key, processing::StatusPtr status, mongo::BSONArray& counts, std::string& msg) {

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

      std::cerr << "X: " << all_regions.size() << "  " << total_size << std::endl;

      std::sort(all_regions.begin(), all_regions.end(),
        [column_name](const RegionPtr & a, const RegionPtr & b) -> bool
          {
            std::string msg;
            int pos_a;
            int pos_b;
            cache::get_column_position_from_dataset(a->dataset_id(), column_name, pos_a, msg);
            cache::get_column_position_from_dataset(b->dataset_id(), column_name, pos_b, msg);
            return a->value(pos_a) <  b->value(pos_b);
          }
      );

      std::cerr << "XYZ" << std::endl;
      for (const auto& x : all_regions) {
        int pos_a;
        cache::get_column_position_from_dataset(x->dataset_id(), column_name, pos_a, msg);

        std::cerr << x->value(pos_a) << std::endl;
      }

      return true;
    }
  }
}