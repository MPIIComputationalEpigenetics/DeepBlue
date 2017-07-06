//
//  distinct_column_values.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 15.06.17.
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
//ter

#include "../datatypes/user.hpp"

#include "../dba/dba.hpp"
#include "../dba/exists.hpp"
#include "../dba/queries.hpp"

#include "../engine/commands.hpp"
#include "../engine/engine.hpp"

#include "../extras/utils.hpp"
#include "../extras/serialize.hpp"

#include "../errors.hpp"

namespace epidb {
  namespace command {

    class DistinctColumnValuesCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::OPERATIONS, "Obtain the distict values of the field.");
      }

      static Parameters parameters_()
      {
        return {
          parameters::QueryId,
          Parameter("field", serialize::STRING, "field that is filtered by"),
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
      DistinctColumnValuesCommand() : Command("distinct_column_values", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        const std::string query_data_id = parameters[0]->as_string();
        const std::string field = parameters[1]->as_string();
        const std::string user_key = parameters[2]->as_string();

        std::string msg;
        datatypes::User user;

        if (!check_permissions(user_key, datatypes::GET_DATA, user, msg )) {
          result.add_error(msg);
          return false;
        }

        if (!dba::exists::query(query_data_id, user_key, msg)) {
          result.add_error("Invalid query id: '" + query_data_id + "'" + msg);
          return false;
        }

        std::string request_id;
        if (!epidb::Engine::instance().queue_distinct(query_data_id, field, user_key, request_id, msg)) {
          result.add_error(msg);
          return false;
        }

        result.add_string(request_id);
        return true;
      }
    } distinctColumnValuesCommand;
  }
}