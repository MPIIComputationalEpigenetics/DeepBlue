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

    bool DatasetCache::retrieve_regions(const std::string& chromosome, std::string& msg)
    {
      mongo::BSONObjBuilder regions_query_builder;
      regions_query_builder.append(dba::KeyMapper::DATASET(), _dataset_id);
      mongo::BSONObj regions_query = regions_query_builder.obj();
      return ::epidb::dba::retrieve::get_regions(_genome, chromosome, regions_query, false, _status, regions, msg) ;
    }

    bool DatasetCache::load_chromosome(const std::string& chromosome, std::string& msg)
    {
      _last_index_position = 0;
      return retrieve_regions(chromosome, msg);
    }

    bool DatasetCache::count_regions(const std::string& chromosome, const Position start, const Position end, size_t& count, std::string& msg)
    {
      count = 0;

      if (chromosome != current_chromosome) {
        load_chromosome(chromosome, msg);
        current_chromosome = chromosome;
      }

      // Walk to the beginning of the regions
      size_t data_size = regions.size();
      Position _actual_position = _last_index_position;
      while (_actual_position < data_size &&
             regions[_actual_position]->end() < start) {
        _actual_position++;
      }
      _last_index_position = _actual_position;

      // Count overlaps
      while (_actual_position < regions.size() &&
             start < regions[_actual_position]->end()  ) {

        if ((regions[_actual_position]->start() < end) &&
            (regions[_actual_position]->end() > start)) {
          count++;
        }
        _actual_position++;
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
