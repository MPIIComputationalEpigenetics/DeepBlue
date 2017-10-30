//
//  merge_queries.cpp.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Fabian Reinartz on 11.09.13.
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

#include "../dba/dba.hpp"
#include "../dba/exists.hpp"
#include "../dba/queries.hpp"
#include "../datatypes/user.hpp"

#include "../engine/commands.hpp"

#include "../extras/serialize.hpp"
#include "../errors.hpp"

namespace epidb {
  namespace command {

    class MergeQueriesCommand : public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::OPERATIONS, "Merge regions from two queries in a new query.");
      }

      static Parameters parameters_()
      {
        Parameter p[] = {
          Parameter("query_a_id", serialize::STRING, "id of the first query"),
          Parameter("query_b_id", serialize::STRING, "id of the second query (or use an array to include multiple queries)", true),
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 3);
        return params;
      }

      static Parameters results_()
      {
        Parameter p[] = {
          Parameter("id", serialize::STRING, "new query id")
        };
        Parameters results(&p[0], &p[0] + 1);
        return results;
      }

    public:
      MergeQueriesCommand() : Command("merge_queries", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        const std::string query_a_id = parameters[0]->as_string();
        const std::string user_key = parameters[2]->as_string();

        std::vector<serialize::ParameterPtr> query_b_ids;
        parameters[1]->children(query_b_ids);


        std::string msg;
        datatypes::User user;

        std::string prev_query  = query_a_id;
        if (!check_permissions(user_key, datatypes::GET_DATA, user, msg )) {
          result.add_error(msg);
          return false;
        }

        if (!dba::exists::query(user, query_a_id, msg)) {
          if (msg.empty()) {
            result.add_error("Invalid first query ID: " + query_a_id);
          } else {
            result.add_error(msg);
          }
          return false;
        }

        for (const serialize::ParameterPtr & query_b_ptr : query_b_ids) {
          std::string query_b_id = query_b_ptr->as_string();

          if (!dba::exists::query(user, query_b_id, msg)) {
            if (msg.empty()) {
              result.add_error("Invalid second query ID: " + query_b_id);
            } else {
              result.add_error(msg);
            }
            return false;
          }

          // TODO: check if the queries are from same genome
          mongo::BSONObjBuilder args_builder;
          args_builder.append("qid_1", prev_query);
          args_builder.append("qid_2", query_b_id);

          if (!dba::query::store_query(user, "merge", args_builder.obj(), prev_query, msg)) {
            result.add_error(msg);
            return false;
          }
        }

        result.add_string(prev_query);
        return true;
      }

    } mergeQueriesCommand;
  }
}
