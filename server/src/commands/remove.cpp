//
//  remove.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 19.07.13.
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

#include <string>

#include "../datatypes/user.hpp"
#include "../dba/users.hpp"
#include "../dba/remove.hpp"
#include "../engine/commands.hpp"
#include "../extras/serialize.hpp"

#include "../errors.hpp"

namespace epidb {
  namespace command {

    class RemoveCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::GENERAL_INFORMATION, "Remove a DeepBlue data.");
      }

      static  Parameters parameters_()
      {
        Parameter p[] = {
          Parameter("id", serialize::STRING, "Data ID to be removed."),
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 2);
        return params;
      }

      static Parameters results_()
      {
        Parameter p[] = {
          Parameter("id", serialize::STRING, "id of the removed data")
        };
        Parameters results(&p[0], &p[0] + 1);
        return results;
      }

    public:
      RemoveCommand() : Command("remove", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        const std::string id = parameters[0]->as_string();
        const std::string user_key = parameters[1]->as_string();

        std::string msg;
        datatypes::User user;

        if (!check_permissions(user_key, datatypes::INCLUDE_ANNOTATIONS, user, msg )) {
          result.add_error(msg);
          return false;
        }

        bool ok = false;
        if (id.compare(0, 1, "a") == 0) {
          ok = dba::remove::annotation(id, user_key, msg);
        } else if (id.compare(0, 1, "g") == 0) {
          ok = dba::remove::genome(id, user_key, msg);
        } else if (id.compare(0, 1, "p") == 0) {
          ok = dba::remove::project(id, user_key, msg);
        } else if (id.compare(0, 2, "bs") == 0) {
          ok = dba::remove::biosource(id, user_key, msg);
        } else if (id.compare(0, 1, "s") == 0) {
          ok = dba::remove::sample(id, user_key, msg);
        } else if (id.compare(0, 2, "em") == 0) {
          ok = dba::remove::epigenetic_mark(id, user_key, msg);
        } else if (id.compare(0, 1, "e") == 0) {
          ok = dba::remove::experiment(id, user_key, msg);
        } else if (id.compare(0, 1, "t") == 0) {
          ok = dba::remove::technique(id, user_key, msg);
        } else if (id.compare(0, 2, "ct") == 0) {
          ok = dba::remove::column_type(id, user_key, msg);
        } else {
          result.add_error(Error::m(ERR_INVALID_IDENTIFIER, id));
          return false;
        }
        if (!ok) {
          result.add_error(msg);
          return false;
        }

        result.add_string(id);
        return true;
      }

    } removeCommand;
  }
}
