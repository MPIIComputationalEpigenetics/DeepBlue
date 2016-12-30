//
//  queries_cache.hpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 26.11.2015.
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

#ifndef EPIDB_QUERIES_CACHE_HPP
#define EPIDB_QUERIES_CACHE_HPP

#include <string>

#include <boost/bimap/set_of.hpp>

#include "../algorithms/lru.hpp"

#include "../datatypes/regions.hpp"

#include "../dba/queries.hpp"

namespace epidb {
  namespace cache {
    namespace query {
      struct QUERY_KEY {
        std::string user_key;
        std::string query_id;
        processing::StatusPtr status;

        int operator<(const QUERY_KEY& rhs) const
        {
          return query_id < rhs.query_id;
        }
      };

      struct QUERY_RESULT {
        bool success;
        ChromosomeRegionsList regions;
        std::string msg;
      };
    }

    bool get_query_cache(const std::string &user_key, const std::string &query_id,
                         processing::StatusPtr status, ChromosomeRegionsList &regions, std::string &msg);

    void queries_cache_invalidate();
  }
}

#endif