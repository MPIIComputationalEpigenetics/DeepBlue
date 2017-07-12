/*
 * Created by Natalie Wirth on 13.06.2015.
 * Copyright (c) 2015 Max Planck Institute for Computer Science. All rights reserved.
 */

#include "../dba/dba.hpp"
#include "../dba/users.hpp"

#include "../datatypes/user.hpp"

#include "../extras/serialize.hpp"

#include "../engine/commands.hpp"

namespace epidb {
  namespace command {

    class UserAuthCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::ADMINISTRATION, "Verify an email-password combination and obtain the user_key.");
      }

      static Parameters parameters_()
      {
        Parameter p[] = {
          Parameter("email", serialize::STRING, "User's email"),
          Parameter("password", serialize::STRING, "User's password"),
        };
        Parameters params(&p[0], &p[0] + 2);
        return params;
      }

      static Parameters results_()
      {
        Parameter p[] = {
          Parameter("user_key", serialize::STRING, "User's user_key")
        };
        Parameters results(&p[0], &p[0] + 1);
        return results;
      }

    public:
      UserAuthCommand() : Command("user_auth", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        const std::string email = parameters[0]->as_string();
        const std::string password = parameters[1]->as_string();

        std::string msg;
        datatypes::User user;

        if (!dba::users::get_user_by_email(email, password, user, msg)) {
          result.add_error(msg);
          return false;
        }

        result.add_string(user.key());
        return true;
      }
    } userAuthCommand;
  }
}
