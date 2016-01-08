//
//  queries_cache.cpp
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
//  Created by Felipe Albrecht on 26.11.2015.
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

#include <string>
#include <map>
#include <unordered_map>

#include <boost/bimap/set_of.hpp>
#include <boost/bimap/unordered_set_of.hpp>

#include "../algorithms/lru.hpp"
#include "../datatypes/regions.hpp"

#include "queries.hpp"
#include "queries_cache.hpp"

namespace epidb {
  namespace dba {
    namespace cache {

      QUERY_RESULT fn(const QUERY_KEY& qk)
      {
        ChromosomeRegionsList regions;
        std::string msg;

        QUERY_RESULT result;
        result.success  =  query::retrieve_query(qk.user_key, qk.query_id, qk.status, result.regions, result.msg);

        return result;
      }

      lru_cache_using_boost<QUERY_KEY, QUERY_RESULT, boost::bimaps::set_of> QUERY_CACHE(fn, 10);


      bool get_query_cache(const std::string &user_key, const std::string &query_id,
                           processing::StatusPtr status, ChromosomeRegionsList &regions, std::string &msg)
      {
        QUERY_KEY qk;
        qk.user_key = user_key;
        qk.query_id = query_id;
        qk.status = status;

        QUERY_RESULT qr = QUERY_CACHE(qk);

        regions = std::move(qr.regions);
        msg = qr.msg;

        return qr.success;
      }
    }
  }
}
