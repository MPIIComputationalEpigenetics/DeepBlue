/*
 * Created by Natalie Wirth on 11.07.2015.
 * Copyright (c) 2015 Max Planck Institute for Computer Science. All rights reserved.
 */

#include <iostream>
#include <sstream>

#include "../dba/dba.hpp"
#include "../dba/users.hpp"
#include "../datatypes/user.hpp"
#include "../entities/users.hpp"
#include "../errors.hpp"
#include "../extras/serialize.hpp"

#include "../engine/commands.hpp"

namespace epidb {
  namespace command {

    class ModifyUserAdminCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::ADMINISTRATION, "Modifies one field of a user.");
      }

      static  Parameters parameters_()
      {
        Parameter p[] = {
          Parameter("user_key", serialize::STRING, "Key of the user to be updated"),
          Parameter("field", serialize::STRING, "Name of field to update"),
          Parameter("value", serialize::STRING, "New value to store in field"),
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 4);
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
      ModifyUserAdminCommand() : Command("modify_user_admin", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        const std::string user_key = parameters[0]->as_string();
        const std::string field = parameters[1]->as_string();
        const std::string value = parameters[2]->as_string();
        const std::string admin_key = parameters[3]->as_string();

        std::string msg;

        datatypes::User admin;
        if (!dba::get_user_by_key(admin_key, admin, msg)) {
          result.add_error(msg);
          return false;
        }

        if(!admin.has_permission(datatypes::ADMIN)) {
          result.add_error(Error::m(ERR_INSUFFICIENT_PERMISSION));
          return false;
        }

        datatypes::User user;
        if (!dba::get_user_by_key(user_key, user, msg)) {
          result.add_error(msg);
          return false;
        }

        if (field == "memory_limit") {
          long long converted;
          std::stringstream ss(value);
          ss >> converted;
          user.set_memory_limit(converted);
        } else if (field == "permission_level") {
          if (value == "ADMIN") {
            user.set_permission_level(datatypes::ADMIN);
          } else if (value == "INCLUDE_COLLECTION_TERMS") {
            user.set_permission_level(datatypes::INCLUDE_COLLECTION_TERMS);
          } else if (value == "INCLUDE_EXPERIMENTS") {
            user.set_permission_level(datatypes::INCLUDE_EXPERIMENTS);
          } else if (value == "INCLUDE_ANNOTATIONS") {
            user.set_permission_level(datatypes::INCLUDE_ANNOTATIONS);
          } else if (value == "GET_DATA") {
            user.set_permission_level(datatypes::GET_DATA);
          } else if (value == "LIST_COLLECTIONS") {
            user.set_permission_level(datatypes::LIST_COLLECTIONS);
          } else if (value == "NONE") {
            user.set_permission_level(datatypes::NONE);
          } else {
            result.add_error("Invalid key");
            return false;
          }
        } else {
          result.add_error("Invalid field name");
          return false;
        }

        if (!dba::modify_user(user, msg)) {
          result.add_error(msg);
          return false;
        }

        result.add_string(user.get_id());
        return true;
      }
    } modifyUserAdminCommand;
  }
}

