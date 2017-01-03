//
//  binning.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 29.12.16.
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
#include "../engine/engine.hpp"

#include "../extras/serialize.hpp"

#include "../errors.hpp"

namespace epidb {
  namespace command {

    class BinningCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::OPERATIONS, "Create set of numbers containing the the count values of the selected data columns");
      }

      static Parameters parameters_()
      {
        return {
          Parameter("query_data_id", serialize::STRING, "query data that will made by the binning."),
          Parameter("column", serialize::STRING, "name of the column that will be used in the aggregation"),
          Parameter("bins", serialize::INTEGER, "number of of bins"),
          parameters::UserKey
        };
      }

      static Parameters results_()
      {
        return {
          Parameter("request_id", serialize::STRING, "Request ID - Use it to retrieve the result with info() and get_request_data()")
        };
      }

    public:
      BinningCommand() : Command("binning", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        const std::string query_data_id = parameters[0]->as_string();
        const std::string column = parameters[1]->as_string();
        const int bars = parameters[2]->as_long();
        const std::string user_key = parameters[3]->as_string();
        std::string msg;
        datatypes::User user;

        if (!check_permissions(user_key, datatypes::GET_DATA, user, msg )) {
          result.add_error(msg);
          return false;
        }

        if (bars <= 0) {
          result.add_error("There must be at least one bin.");
          return false;
        }

        // No more than 65536 bars
        if (bars >= 65536) {
          result.add_error("There must be at no more than 65536 bin.");
          return false;
        }

        if (!dba::exists::query(query_data_id, user_key, msg)) {
          result.add_error(Error::m(ERR_INVALID_QUERY_ID, query_data_id));
          return false;
        }

        std::string request_id;
        if (!epidb::Engine::instance().queue_binning(query_data_id, column, bars, user_key, request_id, msg)) {
          result.add_error(msg);
          return false;
        }

        result.add_string(request_id);
        return true;
      }

    } binningCommand;
  }
}
