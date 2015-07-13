//
//  add_user.cpp
//  epidb
//
//  Created by Felipe Albrecht on 16.06.13.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include "../dba/dba.hpp"
#include "../dba/users.hpp"
#include "../datatypes/user.hpp"
#include "../entities/users.hpp"
#include "../extras/serialize.hpp"

#include "../engine/commands.hpp"

namespace epidb {
  namespace command {

    class AddUserCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::ADMINISTRATION, "Inserts a new user with the given parameters.");
      }

      static  Parameters parameters_()
      {
        Parameter p[] = {
          Parameter("name", serialize::STRING, "name for the new user"),
          Parameter("email", serialize::STRING, "email address of the new user"),
          Parameter("institution", serialize::STRING, "institution of the new user"),
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 4);
        return params;
      }

      static Parameters results_()
      {
        Parameter p[] = {
          Parameter("id", serialize::STRING, "id of the newly inserted user"),
          Parameter("user_key", serialize::STRING, "key of the newly inserted user")
        };
        Parameters results(&p[0], &p[0] + 2);
        return results;
      }

    public:
      AddUserCommand() : Command("add_user", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        const std::string name = parameters[0]->as_string();
        const std::string email = parameters[1]->as_string();
        const std::string institution = parameters[2]->as_string();
        const std::string admin_key = parameters[3]->as_string();

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

        if (!dba::users::is_valid_email(email, msg)) {
          result.add_error(msg);
          return false;
        }
        
        datatypes::User user = datatypes::User(name, email, institution);
        
        user.generate_key();

        if (dba::add_user(user, msg)) {
          result.add_string(user.get_id());
          result.add_string(user.get_key());
          return true;
        } else {
          result.add_error(msg);
          return false;
        }
      }

    } addUserCommand;
  }
}
