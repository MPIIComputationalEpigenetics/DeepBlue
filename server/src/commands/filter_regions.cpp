//
//  filter.cpp
//  epidb
//
//  Created by Felipe Albrecht on 22.09.13.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include "../algorithms/filter.hpp"

#include "../datatypes/user.hpp"

#include "../dba/dba.hpp"
#include "../dba/exists.hpp"
#include "../dba/queries.hpp"

#include "../engine/commands.hpp"

#include "../extras/utils.hpp"
#include "../extras/serialize.hpp"

#include "../errors.hpp"

namespace epidb {
  namespace command {

    class FilterRegionsCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::OPERATIONS, "Filters the result of the given query by the given restrictions.");
      }

      static Parameters parameters_()
      {
        Parameter p[] = {
          Parameter("query_id", serialize::STRING, "id of the query to be filtered"),
          Parameter("field", serialize::STRING, "field that is filtered by"),
          Parameter("operation", serialize::STRING, "operation used for filtering. For 'string' must be '==' or '!=' and for 'number' must be one of these: " + utils::vector_to_string(algorithms::FilterBuilder::operations())),
          Parameter("value", serialize::STRING, "value the operator is applied to"),
          Parameter("type", serialize::STRING, "type of the value: 'number' or 'string' "),
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 6);
        return params;
      }

      static Parameters results_()
      {
        Parameter p[] = {
          Parameter("id", serialize::STRING, "id of filtered query")
        };
        Parameters results(&p[0], &p[0] + 1);
        return results;
      }

    public:
      FilterRegionsCommand() : Command("filter_regions", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        const std::string query_id = parameters[0]->as_string();
        const std::string field = parameters[1]->as_string();
        const std::string operation = parameters[2]->as_string();
        const std::string value = parameters[3]->as_string();
        const std::string type = parameters[4]->as_string();
        const std::string user_key = parameters[5]->as_string();

        std::string msg;
        datatypes::User user;

        if (!check_permissions(user_key, datatypes::GET_DATA, user, msg )) {
          result.add_error(msg);
          return false;
        }

        if (!dba::exists::query(query_id, user_key, msg)) {
          result.add_error("Invalid query id: '" + query_id + "'" + msg);
          return false;
        }

        if (type != "string" && type != "number") {
          result.add_error("Invalid type: '" + type + "'. The type must be 'string' or 'number'");
          return false;
        }

        if (type == "string") {
          result.add_error("Only equals (==) or not equals (!=) are available for type 'string'");
          return false;
        } else {
          if (!algorithms::FilterBuilder::is_valid_operations(operation)) {
            result.add_error("Invalid operation: '" + operation + "'. The operation for type 'number' must be one of these: " + utils::vector_to_string(algorithms::FilterBuilder::operations()));
            return false;
          }
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