//
//  running_cache.hpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 01.11.16.
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

#include <memory>
#include <string>

#include "../dba/retrieve.hpp"
#include "../dba/key_mapper.hpp"

#include "processing.hpp"

#include "running_cache.hpp"

namespace epidb {
  namespace processing {

    bool DatasetCache::load_regions(const std::string& chromosome, const Position start, const Position end, std::string& msg)
    {
      mongo::BSONObjBuilder regions_query_builder;

      regions_query_builder.append(dba::KeyMapper::DATASET(), _dataset_id);
      regions_query_builder.append(dba::KeyMapper::START(), BSON("$lte" << end));
      regions_query_builder.append(dba::KeyMapper::END(), BSON("$gte" << start));

      mongo::BSONObj regions_query = regions_query_builder.obj();
      return ::epidb::dba::retrieve::get_regions(_genome, chromosome, regions_query, false, _status, regions, msg) ;
    }

    bool DatasetCache::count_regions(const std::string& chromosome, const Position start, const Position end, size_t& count, std::string& msg)
    {
      count = 0;

      if ((chromosome != current_chromosome) ||
          end >= _actual_end) {

        // Remove the regions size and counts from status
        _status->subtract_regions(regions.size());
        for (size_t i = 0; i < regions.size(); i++)  {
          _status->subtract_size(regions[i]->size());
        }

        // Load the regions from the actual position until 1 million Bp after the end.
        Position end_load = end + (1000 * 1000);
        if (!load_regions(chromosome, start, end_load, msg)) {
          return false;
        }
        current_chromosome = chromosome;
        _last_index_position = 0;
        _actual_end = end_load;
      }

      // Walk to the beginning of the regions
      size_t data_size = regions.size();
      Position actual_position = _last_index_position;
      while (actual_position < data_size &&
             regions[actual_position]->end() < start) {
        actual_position++;
      }

      _last_index_position = actual_position;

      // Count overlaps
      while (actual_position < regions.size() &&
             start < regions[actual_position]->end()  ) {

        if ((regions[actual_position]->start() < end) &&
            (regions[actual_position]->end() > start)) {
          count++;
        }
        actual_position++;
      }

      return true;
    }

    bool RunningCache::count_regions(const DatasetId id, const std::string& genome,
                                     const std::string& chromosome, const Position start, const Position end, size_t& count,
                                     StatusPtr status, std::string& msg)
    {
      if (caches.find(id) == caches.end()) {
        caches[id] = std::unique_ptr<DatasetCache>(new DatasetCache(id, genome, status));
      }

      return caches[id]->count_regions(chromosome, start, end, count, msg);
    }
  }
}
