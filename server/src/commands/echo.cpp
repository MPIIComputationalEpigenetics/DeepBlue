//
//  echo.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 10.09.13.
//  Copyright (c) 2016 Max Planck Institute for Informatics. All rights reserved.

//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.

//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.

//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
      static CommandDescription desc_()
      {
        return CommandDescription(categories::STATUS, "Echos the server's version.");
      }

      static Parameters parameters_()
      {
        Parameter p[] = {
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0]+1);
        return params;
      }

      static Parameters results_()
      {
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
        utils::IdName user;
        if (!dba::exists::user_by_key(user_key)) {
          user.name = "a Stranger";
        } else {
          if (!dba::users::get_user(user_key, user, msg)) {
            result.add_error(msg);
          }
        }

        std::string echo = "DeepBlue (" + Version::version() + ")" + " says hi to " + user.name;
        result.add_string(echo);
        return true;
      }

    } echoCommand;
  }
}
