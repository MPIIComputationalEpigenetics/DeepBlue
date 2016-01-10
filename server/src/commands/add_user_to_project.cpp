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
        return CommandDescription(categories::PROJECTS, "Include or exclude an user from a project");
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

        std::vector<utils::IdName> user_projects_id_names;
        if (!dba::list::projects(user_key, user_projects_id_names, msg)) {
          result.add_error(msg);
          return false;
        }

        std::vector<std::string> user_projects;
        for (const auto& project : user_projects_id_names) {
          user_projects.push_back(utils::normalize_name(project.name));
        }

        datatypes::Metadata project_res;
        if (!dba::info::get_project(project_id, user_projects, project_res, msg)) {
          result.add_error(msg);
          return false;
        }
        std::string owner = project_res["user"];

        // Getting the project owner ID
        std::string owner_id;
        if (!dba::users::get_id(owner, owner_id, msg)) {
          result.add_error(msg);
          return false;
        }


        std::string user_id;
        if (!dba::users::get_id(user_to_include, user_id, msg)) {
          result.add_error(msg);
          return false;
        }


        // Getting the command operator
        utils::IdName working_user;
        if (!dba::users::get_user(user_key, working_user, msg)) {
          result.add_error(msg);
          return false;
        }

        // Is the command operator admin or project owner ?
        if ((user.is_admin()) && (working_user.id != owner_id)) {
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
