//
//  merge_queries.cpp.cpp
//  epidb
//
//  Created by Fabian Reinartz on 11.09.13.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
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
        return CommandDescription(categories::OPERATIONS, "Merges the regions of the given queries.");
      }

      static Parameters parameters_()
      {
        Parameter p[] = {
          Parameter("query_a_id", serialize::STRING, "id of the first query"),
          Parameter("query_b_id", serialize::STRING, "id of the second query"),
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
        const std::string query_b_id = parameters[1]->as_string();
        const std::string user_key = parameters[2]->as_string();

        std::string msg;
        datatypes::User user;

        if (!check_permissions(user_key, datatypes::GET_DATA, user, msg )) {
          result.add_error(msg);
          return false;
        }

        if (!dba::exists::query(query_a_id, user_key, msg)) {
          if (msg.empty()) {
            result.add_error("Invalid first query ID: " + query_a_id);
          } else {
            result.add_error(msg);
          }
          return false;
        }

        if (!dba::exists::query(query_b_id, user_key, msg)) {
          if (msg.empty()) {
            result.add_error("Invalid second query ID: " + query_b_id);
          } else {
            result.add_error(msg);
          }
          return false;
        }

        // TODO: check if query ids from same genome
        mongo::BSONObjBuilder args_builder;
        args_builder.append("qid_1", query_a_id);
        args_builder.append("qid_2", query_b_id);

        std::string query_id;
        if (!dba::query::store_query("merge", args_builder.obj(), user_key, query_id, msg)) {
          result.add_error(msg);
          return false;
        }

        result.add_string(query_id);
        return true;
      }

    } mergeQueriesCommand;
  }
}
