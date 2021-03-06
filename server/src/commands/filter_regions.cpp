//
//  filter.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 22.09.13.
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
        return CommandDescription(categories::OPERATIONS, "Filter the genomic regions by their content.");
      }

      static Parameters parameters_()
      {
        return {
          parameters::QueryId,
          Parameter("field", serialize::STRING, "field that is filtered by"),
          Parameter("operation", serialize::STRING, "operation used for filtering. For 'string' must be '==' or '!=' and for 'number' must be one of these: " + utils::vector_to_string(algorithms::FilterBuilder::operations())),
          Parameter("value", serialize::STRING, "value the operator is applied to"),
          Parameter("type", serialize::STRING, "type of the value: 'number' or 'string' "),
          parameters::UserKey
        };
      }

      static Parameters results_()
      {
        return {
          Parameter("id", serialize::STRING, "id of filtered query")
        };
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

        if (!dba::exists::query(user, query_id, msg)) {
          result.add_error("Invalid query id: '" + query_id + "'" + msg);
          return false;
        }

        if (type != "string" && type != "number") {
          result.add_error("Invalid type: '" + type + "'. The type must be 'string' or 'number'");
          return false;
        }

        if (type == "string") {
          if (operation != "==" && operation != "!=") {
            result.add_error("It is only possible to filter by equality (==) or differently (!=) in 'string'");
            return false;
          }
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

        std::string filtered_query_id;
        if (!dba::query::store_query(user, "filter", args_builder.obj(), filtered_query_id, msg)) {
          result.add_error(msg);
          return false;
        }

        result.add_string(filtered_query_id);
        return true;
      }

    } filterRegionsCommand;
  }
}