//
//  running_cache.cpp
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

#ifndef EPIDB_RUNNING_CACHE_HPP
#define EPIDB_RUNNING_CACHE_HPP

#include <memory>
#include <string>

namespace epidb {
  namespace processing {

    class Status;
    typedef std::shared_ptr<Status> StatusPtr;

    class DatasetCache {

    private:
      DatasetId _dataset_id;
      std::string _genome;
      StatusPtr _status;
      std::string current_chromosome;
      Regions regions;
      Position _last_index_position;
      Position _actual_end;

      bool retrieve_regions(const std::string& chromosome, std::string& msg);
      bool load_regions(const std::string& chromosome, const Position start, const Position end, std::string& msg);

    public:
      DatasetCache(const DatasetId i, const std::string& g, StatusPtr s):
        _dataset_id(i),
        _genome(g),
        _status(s),
        regions(0),
        _last_index_position(0),
        _actual_end(0) {}

      bool count_regions(const std::string& chromosome, const Position start, const Position end,
                                       size_t& count, std::string& msg);
    };

    class RunningCache {
    private:
      std::unordered_map<DatasetId, std::unique_ptr<DatasetCache>> caches;

    public:
      bool count_regions(const DatasetId id, const std::string& genome,
                                       const std::string& chromosome, const Position start, const Position end, size_t& count,
                                       StatusPtr status, std::string& msg);
    };
  }
}

#endif