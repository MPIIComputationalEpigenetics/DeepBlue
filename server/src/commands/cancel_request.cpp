//
//  cancel_request.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 22.06.15.
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

#include <string>

#include "../datatypes/user.hpp"

#include "../dba/users.hpp"

#include "../engine/commands.hpp"
#include "../engine/engine.hpp"
#include "../mdbq/hub.hpp"

namespace epidb {
  namespace command {

    class CancelRequestCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::GENERAL_INFORMATION, "Stop, cancel, and remove request data. The request processed data is remove if its processing was finished.");
      }

      static  Parameters parameters_()
      {
        Parameter p[] = {
          Parameter("id", serialize::STRING, "Request ID to be canceled, stopped or removed."),
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 2);
        return params;
      }

      static Parameters results_()
      {
        Parameter p[] = {
          Parameter("id", serialize::STRING, "ID of the canceled request")
        };
        Parameters results(&p[0], &p[0] + 1);
        return results;
      }

    public:
      CancelRequestCommand() : Command("cancel_request", parameters_(), results_(), desc_()) {}

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

        if (!epidb::Engine::instance().cancel_request(user, request_id, msg)) {
          result.add_error(msg);
          return false;
        }

        result.add_string(request_id);

        return true;
      }

    } cancelRequestCommand;
  }
}
