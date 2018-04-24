//
//  reprocess.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 10.02.18.
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

#include <iostream>
#include <sstream>
#include <map>

#include "../dba/dba.hpp"
#include "../dba/queries.hpp"
#include "../dba/users.hpp"

#include "../datatypes/user.hpp"

#include "../engine/commands.hpp"
#include "../engine/engine.hpp"

#include "../extras/serialize.hpp"
#include "../extras/utils.hpp"

#include "../errors.hpp"

namespace epidb {
  namespace command {

    class ReprocessCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::REQUESTS, "Reprocess the request. Useful when the request was cancelled or removed.");
      }

      static  Parameters parameters_()
      {
        return {
          Parameter("request_id", serialize::STRING, "ID of the request"),
          parameters::UserKey
        };
      }

      static Parameters results_()
      {
        return {
          Parameter("id", serialize::STRING, "ID of the reprocessed request")
        };
      }

    public:
      ReprocessCommand() : Command("reprocess", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        const std::string request_id = parameters[0]->as_string();
        const std::string user_key = parameters[1]->as_string();

        std::string msg;
        datatypes::User user;

        if (!check_permissions(user_key, datatypes::GET_DATA, user, msg )) {
          result.add_error(msg);
          return false;
        }

        if (!epidb::Engine::instance().user_owns_request(request_id, user.id())) {
          result.add_error("Request ID " + request_id + " not found.");
          return false;
        }

        if (!epidb::Engine::instance().reprocess_request(user, request_id, msg)) {
          result.add_error(msg);
          return false;
        }

        result.add_string(request_id);

        return true;
      }
    } reprocessCommand;
  }
}
