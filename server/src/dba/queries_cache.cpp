//
//  queries_cache.cpp
//  epidb
//
//  Created by Felipe Albrecht on 26.11.2015.
//  Copyright (c) 2015 Max Planck Institute for Computer Science. All rights reserved.
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
