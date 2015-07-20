//
//  init_system.cpp
//  epidb
//
//  Created by Felipe Albrecht on 20.06.13.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include <sstream>

#include "../dba/dba.hpp"
#include "../dba/users.hpp"

#include "../engine/commands.hpp"
#include "../engine/engine.hpp"

#include "../extras/serialize.hpp"

#include "../parser/parser_factory.hpp"

namespace epidb {
  namespace command {

    class InitSystemCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::ADMINISTRATION, "Initializes the system with the given user as administrator.");
      }

      static Parameters parameters_()
      {
        Parameter p[] = {
          Parameter("name", serialize::STRING, "name for the initial user"),
          Parameter("email", serialize::STRING, "email address of the initial user"),
          Parameter("institution", serialize::STRING, "institution of the initial user")
        };
        Parameters params(&p[0], &p[0]+3);
        return params;
      }

      static Parameters results_()
      {
        Parameter p[] = {
          Parameter("admin_key", serialize::STRING, "administrator's authentication key")
        };
        Parameters results(&p[0], &p[0]+1);
        return results;
      }

    public:
      InitSystemCommand() : Command("init_system", parameters_(), results_(), desc_()) {}

      // TODO: Check user
      virtual bool run(const std::string& ip,
                       const serialize::Parameters& parameters, serialize::Parameters& result) const
      {
        const std::string name = parameters[0]->as_string();
        const std::string email = parameters[1]->as_string();
        const std::string institution = parameters[2]->as_string();

        bool r;
        std::string msg;

        if (!check_local(ip, r, msg)) {
          result.add_error(msg);
          return false;
        }
        if (!r) {
          result.add_error("This command can only be executed in the local machine.");
          return false;
        }

        if (dba::is_initialized()) {
          result.add_error("The system was already initialized.");
          return false;
        }

        if (!dba::users::is_valid_email(email, msg)) {
          result.add_error(msg);
          return false;
        }

        std::string user_key;
        if (!dba::init_system(name, email, institution, user_key, msg)) {
          result.add_error(msg);
          return false;
        }

        epidb::Engine::instance().init();

        result.add_string(user_key);
        return true;
      }

    } initSystemCommand;
  }
}
