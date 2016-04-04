//
//  flank.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 04.04.16.
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

    class ExtendCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::OPERATIONS, "Extend regions.");
      }

      static Parameters parameters_()
      {
        Parameter p[] = {
          Parameter("query_id", serialize::STRING, "id of the query that contains the regions"),
          Parameter("length", serialize::INTEGER, "The new region length"),
          Parameter("direction", serialize::STRING, "The direction that the region will be extended: 'BACKWARD', 'FORWARD', 'BOTH'. (Empty value will be used for both direction."),
          Parameter("use_strand", serialize::BOOLEAN, "Use the region column STRAND to define the region direction"),
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 5);
        return params;
      }

      static Parameters results_()
      {
        Parameter p[] = {
          Parameter("id", serialize::STRING, "id of the new query")
        };
        Parameters results(&p[0], &p[0] + 1);
        return results;
      }

    public:
      ExtendCommand() : Command("extend", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        const std::string flanked_query_id = parameters[0]->as_string();
        const int length = parameters[1]->isNull() ? -1 : parameters[1]->as_long();
        const std::string direction = parameters[2]->as_string();
        const bool use_strand = parameters[3]->as_boolean();
        const std::string user_key = parameters[4]->as_string();

        std::string msg;
        datatypes::User user;

        if (!check_permissions(user_key, datatypes::GET_DATA, user, msg )) {
          result.add_error(msg);
          return false;
        }

        if (!dba::exists::query(flanked_query_id, user_key, msg)) {
          if (msg.empty()) {
            result.add_error(Error::m(ERR_INVALID_QUERY_ID, flanked_query_id));
          } else {
            result.add_error(msg);
          }
          return false;
        }

        // TODO: Check for negative length

        mongo::BSONObjBuilder args_builder;
        args_builder.append("query_id", flanked_query_id);
        args_builder.append("length", length);
        args_builder.append("direction", utils::normalize_name(direction));
        args_builder.append("use_strand", use_strand);

        std::string query_id;
        if (!dba::query::store_query("extend", args_builder.obj(), user_key, query_id, msg)) {
          result.add_error(msg);
          return false;
        }

        result.add_string(query_id);
        return true;
      }

    } extendCommand;
  }
}
