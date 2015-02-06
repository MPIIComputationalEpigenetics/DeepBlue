//
//  count_regions.cpp
//  epidb
//
//  Created by Felipe Albrecht on 28.01.14.
//  Copyright (c) 2015 Max Planck Institute for Computer Science. All rights reserved.
//

#include <string>

#include "../dba/queries.hpp"

namespace epidb {
  namespace processing {

    bool count_regions(const std::string &query_id, const std::string &user_key, size_t &count, std::string &msg)
    {
      if (!dba::query::count_regions(query_id, user_key, count, msg)) {
        return false;
      }
      return true;
    }

  }
}