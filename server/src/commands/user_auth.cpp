/*
 * Created by Natalie Wirth on 13.06.2015.
 * Copyright (c) 2015 Max Planck Institute for Computer Science. All rights reserved.
 */

#include "../dba/dba.hpp"
#include "../dba/users.hpp"
#include "../datatypes/user.hpp"
#include "../entities/users.hpp"
#include "../extras/serialize.hpp"

#include "../engine/commands.hpp"

namespace epidb {
  namespace command {

    class UserAuthCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::ADMINISTRATION, "Checks email-password combination.");
      }

      static Parameters parameters_()
      {
        Parameter p[] = {
          Parameter("email", serialize::STRING, "Email of a user"),
          Parameter("password", serialize::STRING, "Password of a user"),
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 3);
        return params;
      }

      static Parameters results_()
      {
        Parameter p[] = {
          Parameter("user_key", serialize::STRING, "Key of the user")
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
        const std::string admin_key = parameters[2]->as_string();

        std::string msg;

        datatypes::User admin;
        if (!dba::get_user_by_key(admin_key, admin, msg)) {
          result.add_error(msg);
          return false;
        }
        
        if (!admin.has_permission(datatypes::PermissionLevel::ADMIN)) {
          result.add_error("The given key is not an admin-key");
          return false;
        }
                
        datatypes::User user;
        if (!dba::get_user_by_email(email, user, msg)) {
            result.add_error(msg);
            return false;
        }
        
        if (user.get_password() == password) {
            result.add_string(user.get_key());
            return true;
        } else {
            result.add_error("Email-password combination does not exist");
        }
        return false;
      }
    } userAuthCommand;
  }
}
