//
//  bind_user.cpp
//  epidb
//
//  Created by Felipe Albrecht on 03.11.14.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include "../dba/dba.hpp"
#include "../dba/users.hpp"

#include "../extras/serialize.hpp"

#include "../engine/commands.hpp"

namespace epidb {
    namespace command {

      class BindUserCommand: public Command {

      private:
        static CommandDescription desc_() {
          return CommandDescription(categories::ADMINISTRATION, "Bind User and Password with a User Key.");
        }

        static  Parameters parameters_() {
          Parameter p[] = {
            Parameter("email", serialize::STRING, "user email - used for registration"),
            Parameter("password", serialize::STRING, "user password - used for front end"),
            Parameter("user_key", serialize::STRING, "user key - used for API access "),
            parameters::UserKey
          };
          Parameters params(&p[0], &p[0]+4);
          return params;
        }

        static Parameters results_() {
          Parameter p[] = {
            Parameter("password", serialize::STRING, "user password")
          };
          Parameters results(&p[0], &p[0]+1);
          return results;
        }

      public:
        BindUserCommand() : Command("bind_user", parameters_(), results_(), desc_()) {}

        virtual bool run(const std::string& ip,
            const serialize::Parameters& parameters, serialize::Parameters& result) const
        {
          const std::string email = parameters[0]->as_string();
          const std::string password = parameters[1]->as_string();
          const std::string user_key = parameters[2]->as_string();
          const std::string admin_key = parameters[3]->as_string();

          std::string msg;

          bool is_initialized;
          if (!dba::is_initialized(is_initialized, msg)) {
            result.add_error(msg);
            return false;
          }
          if (!is_initialized) {
            result.add_error("The system was not initialized.");
            return false;
          }


          bool is_admin_key;
          if (!dba::users::is_admin_key(admin_key, is_admin_key, msg)) {
            result.add_error(msg);
            return false;
          }
          if (!is_admin_key) {
            result.add_error("The given key is not an admin-key");
            return false;
          }

          if (!dba::users::is_valid_email(email, msg)) {
            result.add_error("The given email is not registered into DeepBlue");
            return false;
          }

          /*
          std::string id;
          if (dba::users::bind_user(email, password, user_key, admin_key)) {
            result.add_string(id);
            result.add_string(key);
            return true;
          } else {
            result.add_error(msg);
            return false;
          }
          */

          return true;
        }

      } bindUserCommand;
  }
}
