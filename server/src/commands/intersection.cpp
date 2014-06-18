//
//  intersections.cpp
//  epidb
//
//  Created by Fabian Reinartz on 29.08.13.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include "../dba/dba.hpp"
#include "../dba/queries.hpp"

#include "../engine/commands.hpp"

#include "../extras/serialize.hpp"

namespace epidb {
    namespace command {

      class IntersectionCommand: public Command {

      private:
        static CommandDescription desc_() {
          return CommandDescription(categories::OPERATIONS, "Select regions from the first query that does intersect with at least one second query region.");
        }

        static Parameters parameters_() {
          Parameter p[] = {
            Parameter("query_a_id", serialize::STRING, "id of the first query"),
            Parameter("query_b_id", serialize::STRING, "id of the second query"),
            parameters::UserKey
          };
          Parameters params(&p[0], &p[0]+3);
          return params;
        }

        static Parameters results_() {
          Parameter p[] = {
            Parameter("id", serialize::STRING, "id of the new query")
          };
          Parameters results(&p[0], &p[0]+1);
          return results;
        }

      public:
        IntersectionCommand() : Command("intersection", parameters_(), results_(), desc_()) {}

        virtual bool run(const std::string& ip,
            const serialize::Parameters& parameters, serialize::Parameters& result) const
        {
          const std::string query_a_id = parameters[0]->as_string();
          const std::string query_b_id = parameters[1]->as_string();
          const std::string user_key = parameters[2]->as_string();

          std::string msg;
          bool ok;
          if (!dba::check_user(user_key, ok, msg)) {
            result.add_error(msg);
            return false;
          }
          if (!ok) {
            result.add_error("Invalid user key.");
            return false;
          }

          // check query ids
          if (!dba::check_query(user_key, query_a_id, ok, msg)) {
            result.add_error(msg);
            return false;
          }
          if (!ok) {
            result.add_error("Invalid first query id.");
            return false;
          }

          if (!dba::check_query(user_key, query_b_id, ok, msg)) {
            result.add_error(msg);
            return false;
          }
          if (!ok) {
            result.add_error("Invalid second query id.");
            return false;
          }

          mongo::BSONObjBuilder args_builder;
          args_builder.append("qid_1", query_a_id);
          args_builder.append("qid_2", query_b_id);

          std::string query_id;
          if (!dba::query::store_query("intersect", args_builder.obj(), user_key, query_id, msg)) {
            result.add_error(msg);
            return false;
          }

          result.add_string(query_id);
          return true;
        }

      } intersectionCommand;
  }
}



