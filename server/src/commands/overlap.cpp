//
//  overlap.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 20.12.16.
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

    class OverlapCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::OPERATIONS, "Select genomic regions that overlap or not overlap with with the specified number of regions of the second query. Important: This command is still experimental and changes may occour.");
      }

      static Parameters parameters_()
      {
        return {
          Parameter("query_data_id", serialize::STRING, "query data that will be filtered."),
          Parameter("query_filter_id", serialize::STRING, "query containing the regions that the regions of the query_data_id must overlap."),
          Parameter("overlap", serialize::BOOLEAN, "True if must overlap, or false if must not overlap."),
          Parameter("amount", serialize::INTEGER, "Amount of regions that must overlap. Use the parameter 'amount_type' ('bp'' or '%') to specify the unit.  For example, use the value '10' with the amount_type '%' to specify that 10% of the bases in both regions must overlap, or use '10' with the amount_type 'bp' to specify that at least 10 bases must or must not overlap."),
          Parameter("amount_type", serialize::STRING, "Type of the amount: 'bp' for base pairs and '%' for percentage. "),
          parameters::UserKey
        };
      }

      static Parameters results_()
      {
        return {
          Parameter("id", serialize::STRING, "id of the new query")
        };
      }

    public:
      OverlapCommand() : Command("overlap", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        const std::string query_a_id = parameters[0]->as_string();
        const std::string query_b_id = parameters[1]->as_string();
        const bool overlap = parameters[2]->as_boolean();
        const double amount = parameters[3]->as_number();
        const std::string amount_type = parameters[4]->as_string();
        const std::string user_key = parameters[5]->as_string();

        std::string msg;
        datatypes::User user;

        if (!check_permissions(user_key, datatypes::GET_DATA, user, msg )) {
          result.add_error(msg);
          return false;
        }

        if (!dba::exists::query(query_a_id, user_key, msg)) {
          if (msg.empty()) {
            result.add_error(Error::m(ERR_INVALID_QUERY_ID, query_a_id));
          } else {
            result.add_error(msg);
          }
          return false;
        }

        if (!dba::exists::query(query_b_id, user_key, msg)) {
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
        args_builder.append("overlap", overlap);
        args_builder.append("amount", amount);
        args_builder.append("amount_type", amount_type);

        std::string query_id;
        if (!dba::query::store_query("overlap", args_builder.obj(), user_key, query_id, msg)) {
          result.add_error(msg);
          return false;
        }

        result.add_string(query_id);
        return true;
      }

    } overlapCommand;
  }
}
