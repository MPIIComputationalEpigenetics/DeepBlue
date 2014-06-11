//
//  filter.cpp
//  epidb
//
//  Created by Felipe Albrecht on 22.09.13.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include <sstream>
#include <map>

#include "../dba/dba.hpp"
#include "../dba/queries.hpp"

#include "../engine/commands.hpp"

#include "../extras/utils.hpp"
#include "../extras/serialize.hpp"

namespace epidb {
    namespace command {

      class FilterRegionsCommand: public Command {

      private:
        static CommandDescription desc_() {
          return CommandDescription(categories::OPERATIONS, "Filters the result of the given query by the given restrictions.");
        }

        static Parameters parameters_() {
          Parameter p[] = {
            Parameter("query_id", serialize::STRING, "id of the query to be filtered"),
            Parameter("field", serialize::STRING, "field that is filtered by"),
            Parameter("operation", serialize::STRING, "operation used for filtering"),
            Parameter("value", serialize::STRING, "value the operator is applied to"),
            Parameter("type", serialize::STRING, "type of the value"),
            parameters::UserKey
          };
          Parameters params(&p[0], &p[0]+6);
          return params;
        }

        static Parameters results_() {
          Parameter p[] = {
            Parameter("id", serialize::STRING, "id of filtered query")
          };
          Parameters results(&p[0], &p[0]+1);
          return results;
        }

      public:
        FilterRegionsCommand() : Command("filter_regions", parameters_(), results_(), desc_()) {}

        virtual bool run(const std::string& ip,
            const serialize::Parameters& parameters, serialize::Parameters& result) const
        {
          const std::string query_id = parameters[0]->as_string();
          const std::string field = parameters[1]->as_string();
          const std::string operation = parameters[2]->as_string();
          const std::string value = parameters[3]->as_string();
          const std::string type = parameters[4]->as_string();
          const std::string user_key = parameters[5]->as_string();

          std::string msg;
          bool ok = false;
          if (!dba::check_user(user_key, ok, msg)) {
            result.add_error(msg);
            return false;
          }
          if (!ok) {
            result.add_error("Invalid user key.");
            return false;
          }

          if (!dba::check_query(user_key, query_id, ok, msg)) {
            result.add_error(msg);
            return false;
          }
          if (!ok) {
            result.add_error("Invalid query id.");
            return false;
          }

          mongo::BSONObjBuilder args_builder;
          args_builder.append("query", query_id);
          args_builder.append("field", field);
          args_builder.append("operation", operation);
          args_builder.append("value", value);
          args_builder.append("type", type);

          std::string fquery_id;
          if (!dba::query::store_query("filter", args_builder.obj(), user_key, fquery_id, msg)) {
            result.add_error(msg);
            return false;
          }

          result.add_string(fquery_id);
          return true;
        }

      } filterRegionsCommand;
    }
}