//
//  queries_cache.hpp
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

namespace epidb {
  namespace dba {
    namespace cache {
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


      bool get_query_cache(const std::string &user_key, const std::string &query_id,
                           processing::StatusPtr status, ChromosomeRegionsList &regions, std::string &msg);
    }
  }
}