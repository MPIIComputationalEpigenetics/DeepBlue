//
//  queries_cache.cpp
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

#include <condition_variable>
#include <mutex>
#include <string>

#include <boost/bimap/set_of.hpp>

#include "../algorithms/lru.hpp"

#include "../datatypes/regions.hpp"

#include "../dba/queries.hpp"

#include "queries_cache.hpp"

namespace epidb {
  namespace cache {

    std::condition_variable cv;
    std::set<std::string> waiting_list;
    std::mutex m;

    query::QUERY_RESULT fn(const query::QUERY_KEY& qk)
    {
      query::QUERY_RESULT result;
      result.success = dba::query::retrieve_query(qk.user, qk.query_id, qk.status, result.regions, result.msg);

      return result;
    }

    lru_cache_using_boost<query::QUERY_KEY, query::QUERY_RESULT, boost::bimaps::set_of> QUERY_CACHE(fn, 32);


    bool get_query_cache(const datatypes::User& user,
                         const std::string &query_id,
                         processing::StatusPtr status, ChromosomeRegionsList &regions, std::string &msg)
    {
      query::QUERY_KEY qk;
      qk.user = user;
      qk.query_id = query_id;
      qk.status = status;

      // this query is being processed
      if (waiting_list.find(qk.query_id) != waiting_list.end()) {
        std::unique_lock<std::mutex> lk(m);
        cv.wait(lk, [qk] {
          return waiting_list.find(qk.query_id) == waiting_list.end();
        });
      }

      {
        std::lock_guard<std::mutex> guard(m);
        waiting_list.insert(qk.query_id);
      }
      query::QUERY_RESULT qr = QUERY_CACHE(qk);
      {
        std::lock_guard<std::mutex> guard(m);
        waiting_list.erase(qk.query_id);
        cv.notify_all();
      }

      regions = std::move(qr.regions);
      msg = qr.msg;

      return qr.success;
    }

    void queries_cache_invalidate()
    {
      QUERY_CACHE.clear();
    }
  }
}

