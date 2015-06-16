/*
 * Created by Natalie Wirth on 13.06.2015.
 * Copyright (c) 2015 Max Planck Institute for Computer Science. All rights reserved.
 */

#include <iostream>

#include "../dba/dba.hpp"
#include "../dba/users.hpp"
#include "../datatypes/user.hpp"
#include "../entities/users.hpp"
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
          Parameter("user_key", serialize::STRING, "Name of field to update"),
          Parameter("field", serialize::STRING, "Name of field to update"),
          Parameter("value", serialize::STRING, "New value to store in field"),
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 4);
        return params;
      }

      static Parameters results_()
      {
        Parameter p[] = {};
        Parameters results(&p[0], &p[0] + 0);
        return results;
      }

    public:
      ModifyUserCommand() : Command("modify_user", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        const std::string key_change = parameters[0]->as_string();
        const std::string field = parameters[1]->as_string();
        const std::string value = parameters[2]->as_string();
        const std::string key = parameters[3]->as_string();

        std::string msg;
        
        bool allowed = false;
        if (key == key_change) {
            allowed = true;
        } else {
            datatypes::User admin;
            if (!dba::get_user_by_key(key, admin, msg)) {
              result.add_error(msg);
              return false;
            }
            
            if (admin.has_permission(datatypes::PermissionLevel::ADMIN)) {
                allowed = true;
            }
        }
        
        if (allowed) {
            datatypes::User user;
            if (!dba::get_user_by_key(key_change, user, msg)){
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
            } else if (field == "memory_limit") {
                // TODO
            } else {
                result.add_error("Invalid field name");
                return false;
            }
            
            if(!dba::add_user(user, msg)){
                result.add_error(msg);
                return false;
            }
        } else {
            result.add_error("Insufficient permission");
            return false;
        }
        return true;
      }
    } modifyUserCommand;
  }
}
