//
//  get_experiments_by_query.cpp
//  epidb
//
//  Created by Felipe Albrecht on 28.01.14.
//  Copyright (c) 2016 Max Planck Institute for Informatics. All rights reserved.
//

#include <string>
#include <vector>

#include "../dba/queries.hpp"

#include "../extras/utils.hpp"

namespace epidb {
  namespace processing {

    bool get_experiments_by_query(const std::string &query_id, const std::string &user_key, StatusPtr status, std::vector<utils::IdName> &experiments, std::string &msg)
    {

      if (!dba::query::get_experiments_by_query(user_key, query_id, status, experiments, msg)) {
        return false;
      }

      return true;
    }
  }
}