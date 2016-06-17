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

#include <map>
#include <sstream>
#include <iostream>

#include "../datatypes/user.hpp"

#include "../dba/dba.hpp"
#include "../dba/queries.hpp"

#include "../engine/commands.hpp"
#include "../engine/engine.hpp"

#include "../extras/serialize.hpp"

#include "../errors.hpp"

namespace epidb {
  namespace command {

    class QueryCacheCommand : public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::OPERATIONS, "Cache a query result in DeepBlue memory. This command is useful when the same query ID is used multiple times in different requests. The command is an advice for DeepBlue to cache the query result and there is no guarantee that this query data access will be faster.");
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
        if (!check_permissions(user_key, datatypes::GET_DATA, user, msg )) {
          result.add_error(msg);
          return false;
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
