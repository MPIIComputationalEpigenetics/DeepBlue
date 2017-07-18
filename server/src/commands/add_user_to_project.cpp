//
//  add_user_to_project.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 28.05.15.
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

#include "../datatypes/metadata.hpp"
#include "../datatypes/projects.hpp"

#include "../dba/dba.hpp"
#include "../dba/info.hpp"
#include "../dba/users.hpp"
#include "../dba/list.hpp"
#include "../datatypes/user.hpp"
#include "../engine/commands.hpp"

#include "../extras/serialize.hpp"
#include "../extras/utils.hpp"

#include "../errors.hpp"

namespace epidb {
  namespace command {

    class AddUserToProjectCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::PROJECTS, "Add a user as Project member.");
      }

      static  Parameters parameters_()
      {
        Parameter p[] = {
          Parameter("user", serialize::STRING, "User name or ID"),
          Parameter("project", serialize::STRING, "Project name or ID"),
          Parameter("set", serialize::BOOLEAN, "True to include the user or false to remove"),
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 4);
        return params;
      }

      static Parameters results_()
      {
        Parameter p[] = {
          Parameter("user_id", serialize::STRING, "id of the user"),
          Parameter("project_id", serialize::STRING, "id of the project"),
        };
        Parameters results(&p[0], &p[0] + 1);
        return results;
      }

    public:
      AddUserToProjectCommand() : Command("add_user_to_project", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        const std::string user_to_include = parameters[0]->as_string();
        const std::string project = parameters[1]->as_string();
        const bool include = parameters[2]->as_boolean();
        const std::string user_key = parameters[3]->as_string();

        std::string msg;
        datatypes::User user;

        if (!check_permissions(user_key, datatypes::INCLUDE_EXPERIMENTS, user, msg )) {
          result.add_error(msg);
          return false;
        }

        std::string project_id;
        if (!datatypes::projects::get_id(project, project_id, msg)) {
          result.add_error(msg);
          return false;
        }


        datatypes::Metadata project_res;
        if (!dba::info::get_project(user, project_id, project_res, msg)) {
          result.add_error(msg);
          return false;
        }
        std::string owner_id = project_res["user"];

        std::string user_id;
        if (!dba::users::get_id(user_to_include, user_id, msg)) {
          result.add_error(msg);
          return false;
        }

        // Is the command operator admin or project owner ?
        if ((!user.is_admin()) && // Is not ADMIN
            (user.id() != owner_id)) { // Neither the owner
          result.add_error(Error::m(ERR_PERMISSION_PROJECT, project));
          return false;
        }

        // Execute the command
        if (!datatypes::projects::add_user_to_project(user_id, project_id, include, msg)) {
          result.add_error(msg);
          return false;
        }

        result.add_string(user_id);
        result.add_string(project_id);
        return true;
      }

    } addUserToProject;
  }
}
