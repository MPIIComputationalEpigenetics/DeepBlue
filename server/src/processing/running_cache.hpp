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
      std::string _genome;
      StatusPtr _status;
      std::string current_chromosome;
      std::string _sequence;

      bool load_sequence(const std::string& chromosome, std::string& msg);

    public:
      DatasetCache(const std::string& g, StatusPtr s):
        _genome(g),
        _status(s),
        _sequence() {}

      bool count_regions(const std::string& pattern,
                         const std::string& chromosome, const Position start, const Position end,
                         size_t& count, std::string& msg);

      bool get_sequence(const std::string& chromosome, const Position start, const Position end,
                        std::string& sequence, std::string& msg);
    };

    class RunningCache {
    private:
      std::unordered_map<std::string, std::unique_ptr<DatasetCache>> caches;

    public:
      bool count_regions(const std::string& genome, const std::string& chromosome, const std::string& pattern,
                         const Position start, const Position end, size_t& count,
                         StatusPtr status, std::string& msg);

      bool get_sequence(const std::string & genome, const std::string & chromosome,
                                    const Position start, const Position end, std::string & sequence,
                                    StatusPtr status, std::string & msg);
    };
  }
}

#endif