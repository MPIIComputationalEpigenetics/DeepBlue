//
//  cancel_request.cpp
//  epidb
//
//  Created by Felipe Albrecht on 22.06.15.
//  Copyright (c) 2013,2014,2015 Max Planck Institute for Computer Science. All rights reserved.
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
        return CommandDescription(categories::GENERAL_INFORMATION, "Stop, cancel, and remove request data. Its data will be remove if the request did finish.");
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
        const std::string id = parameters[0]->as_string();
        const std::string user_key = parameters[1]->as_string();

        std::string msg;
        datatypes::User user;

        if (!check_permissions(user_key, datatypes::GET_DATA, user, msg )) {
          result.add_error(msg);
          return false;
        }

        if (!epidb::Engine::instance().cancel_request(id, msg)) {
          result.add_error(msg);
          return false;
        }

        return true;
      }

    } cancelRequestCommand;
  }
}
