//
//  queries_cache.cpp
//  epidb
//
//  Created by Felipe Albrecht on 26.11.2015.
//  Copyright (c) 2015 Max Planck Institute for Computer Science. All rights reserved.
//

#include <map>
#include <sstream>
#include <iostream>

#include "../datatypes/user.hpp"

#include "../dba/dba.hpp"
#include "../dba/queries.hpp"

#include "../engine/commands.hpp"
#include "../engine/engine.hpp"
#include "../engine/request.hpp"

#include "../extras/serialize.hpp"

#include "../errors.hpp"

namespace epidb {
  namespace command {

    class QueryCacheCommand : public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::OPERATIONS,
                                  "Return information for the given ID (or IDs).");
      }

      static  Parameters parameters_()
      {
        Parameter p[] = {
          Parameter("query_id", serialize::STRING, "query ID"),
          Parameter("cache", serialize::BOOLEAN, "set or unset this query caching"),
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 3);
        return params;
      }

      static Parameters results_()
      {
        Parameter p[] = {
          Parameter("information", serialize::STRING, "New query ID.")
        };
        Parameters results(&p[0], &p[0] + 1);
        return results;
      }

    public:
      QueryCacheCommand() : Command("query_cache", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        const std::string query_id = parameters[0]->as_string();
        const bool b_type = parameters[1]->as_boolean();
        const std::string user_key = parameters[2]->as_string();

        std::string msg;
        datatypes::User user;

        std::string err_msg;
        bool has_list_collections_permissions = false;
        if (check_permissions(user_key, datatypes::GET_DATA, user, msg )) {
          has_list_collections_permissions = true;
        } else {
          err_msg = msg;
        }

        std::string type;
        if (b_type) {
          type = "yes";
        } else {
          type = "no";
        }

        std::string new_query_id;
        if (!dba::query::modify_query(query_id, "cache", type, user_key, new_query_id, msg)) {
          result.add_error(msg);
          return false;
        }

        result.add_string(new_query_id);
        return true;
      }
    } queryCacheCommand;
  }
}
