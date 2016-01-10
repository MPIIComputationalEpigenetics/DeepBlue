//
//  add_user.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 16.06.13.
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

#include "../dba/dba.hpp"
#include "../dba/users.hpp"
#include "../datatypes/user.hpp"
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
        if (!check_permissions(admin_key, datatypes::ADMIN, admin, msg )) {
          result.add_error(msg);
          return false;
        }

        if (!dba::users::is_valid_email(email, msg)) {
          result.add_error(msg);
          return false;
        }

        datatypes::User new_user = datatypes::User(name, email, institution);

        new_user.generate_key();

        if (dba::users::add_user(new_user, msg)) {
          result.add_string(new_user.get_id());
          result.add_string(new_user.get_key());
          return true;
        } else {
          result.add_error(msg);
          return false;
        }
      }

    } addUserCommand;
  }
}
