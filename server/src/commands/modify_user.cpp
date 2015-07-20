/*
 * Created by Natalie Wirth on 13.06.2015.
 * Copyright (c) 2015 Max Planck Institute for Computer Science. All rights reserved.
 */

#include <iostream>
#include <sstream>

#include "../dba/dba.hpp"
#include "../dba/users.hpp"

#include "../datatypes/user.hpp"

#include "../extras/serialize.hpp"

#include "../engine/commands.hpp"

namespace epidb {
  namespace command {

    class ModifyUserCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::ADMINISTRATION, "Modifies one field of a user.");
      }

      static  Parameters parameters_()
      {
        Parameter p[] = {
          Parameter("field", serialize::STRING, "Name of field to update"),
          Parameter("value", serialize::STRING, "New value to store in field"),
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 3);
        return params;
      }

      static Parameters results_()
      {
        Parameter p[] = {
          Parameter("ID", serialize::STRING, "ID of the modified user"),
        };
        Parameters results(&p[0], &p[0] + 0);
        return results;
      }

    public:
      ModifyUserCommand() : Command("modify_user", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        const std::string field = parameters[0]->as_string();
        const std::string value = parameters[1]->as_string();
        const std::string user_key = parameters[2]->as_string();

        std::string msg;

        datatypes::User user;
        if (!dba::users::get_user_by_key(user_key, user, msg)) {
          result.add_error(msg);
          return false;
        }
        if (field == "email") {
          user.set_email(value);
        } else if (field == "name") {
          user.set_name(value);
        } else if (field == "password") {
          user.set_password(value);
        } else if (field == "institution") {
          user.set_institution(value);
        } else {
          result.add_error("Invalid field name");
          return false;
        }

        if (!dba::users::modify_user(user, msg)) {
          result.add_error(msg);
          return false;
        }

        result.add_string(user.get_id());
        return true;
      }
    } modifyUserCommand;
  }
}
