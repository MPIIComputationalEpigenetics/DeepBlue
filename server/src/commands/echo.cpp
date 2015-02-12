//
//  echo.cpp
//  epidb
//
//  Created by Felipe Albrecht on 10.09.13.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include "../version.hpp"
#include "../dba/dba.hpp"
#include "../dba/exists.hpp"
#include "../dba/users.hpp"
#include "../extras/serialize.hpp"

#include "../engine/commands.hpp"

namespace epidb {
    namespace command {

      class EchoCommand: public Command {

      private:
        static CommandDescription desc_() {
          return CommandDescription(categories::STATUS, "Echos the server's version.");
        }

        static Parameters parameters_() {
          Parameter p[] = {
            parameters::UserKey
          };
          Parameters params(&p[0], &p[0]+1);
          return params;
        }

        static Parameters results_() {
          Parameter p[] = {
            Parameter("message", serialize::STRING, "echo message including version")
          };
          Parameters results(&p[0], &p[0]+1);
          return results;
        }

      public:
        EchoCommand() : Command("echo", parameters_(), results_(), desc_()) {}

        virtual bool run(const std::string& ip,
            const serialize::Parameters& parameters, serialize::Parameters& result) const
        {
          const std::string user_key = parameters[0]->as_string();

          std::string msg;
          std::string name;
          if (!dba::exists::user(user_key)) {
            name = "a Stranger";
          } else {
            if (!dba::users::get_user_name(user_key, name, msg)) {
              result.add_error(msg);
            }
          }

          std::string echo = "DeepBlue (" + Version::version() + ")" + " says hi to " + name;
          result.add_string(echo);
          return true;
        }

      } echoCommand;
  }
}
