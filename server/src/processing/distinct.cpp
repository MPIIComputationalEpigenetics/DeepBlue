//
//  distinct.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 15.06.17.
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
#include <unordered_map>

#include "../cache/column_dataset_cache.hpp"

#include "../dba/queries.hpp"

#include "processing.hpp"

namespace epidb {
  namespace processing {
    bool distinct(const std::string& query_id, const std::string& column_name, const std::string& user_key, processing::StatusPtr status, mongo::BSONObj& result, std::string& msg)
    {
      INIT_PROCESSING(PROCESS_DISTINCT, status)

      ChromosomeRegionsList chromosomeRegionsList;
      if (!dba::query::retrieve_query(user_key, query_id, status, chromosomeRegionsList, msg)) {
        return false;
      }

      size_t total_size = 0;
      for (const auto& chromosomeRegions: chromosomeRegionsList) {
        total_size += chromosomeRegions.second.size();
      }

      Regions all_regions(total_size);

      std::unordered_map<std::string, size_t> counter;

      for (const auto& chromosomeRegions: chromosomeRegionsList) {
        Regions regions = std::move(chromosomeRegions.second);

        for (const RegionPtr& region: regions) {
          int column_pos;
          if (!cache::get_column_position_from_dataset(region->dataset_id(), column_name, column_pos, msg)) {
            return false;
          }
          std::string value = region->get_string(column_pos);

          counter[value]++;
        }
      }

      mongo::BSONObjBuilder bob;
      for (const auto& elem: counter) {
        bob.append(elem.first, (long long) elem.second);
      }

      result = bob.obj();

      return true;
    }
  }
}