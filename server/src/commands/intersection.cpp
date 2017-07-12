//
//  intersections.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Fabian Reinartz on 29.08.13.
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

    class IntersectionCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::OPERATIONS, "Select genomic regions that intersect with at least one region of the second query. This command is a simplified version of the 'overlap' command.");
      }

      static Parameters parameters_()
      {
        Parameter p[] = {
          Parameter("query_data_id", serialize::STRING, "query data that will be filtered."),
          Parameter("query_filter_id", serialize::STRING, "query containing the regions that the regions of the query_data_id must overlap."),
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 3);
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
      IntersectionCommand() : Command("intersection", parameters_(), results_(), desc_()) {}

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

        if (!dba::exists::query(user, query_a_id, msg)) {
          if (msg.empty()) {
            result.add_error(Error::m(ERR_INVALID_QUERY_ID, query_a_id));
          } else {
            result.add_error(msg);
          }
          return false;
        }

        if (!dba::exists::query(user, query_b_id, msg)) {
          if (msg.empty()) {
            result.add_error(Error::m(ERR_INVALID_QUERY_ID, query_b_id));
          } else {
            result.add_error(msg);
          }
          return false;
        }

        mongo::BSONObjBuilder args_builder;
        args_builder.append("qid_1", query_a_id);
        args_builder.append("qid_2", query_b_id);

        std::string query_id;
        if (!dba::query::store_query(user, "intersect", args_builder.obj(), query_id, msg)) {
          result.add_error(msg);
          return false;
        }

        result.add_string(query_id);
        return true;
      }

    } intersectionCommand;
  }
}
