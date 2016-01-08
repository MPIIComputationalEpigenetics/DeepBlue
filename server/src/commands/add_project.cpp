//
//  add_project.cpp
//  DeepBlue Epigenomic Data Server

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
//  Created by Felipe Albrecht on 16.06.13.
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

#include "../datatypes/projects.hpp"

#include "../dba/dba.hpp"
#include "../dba/users.hpp"
#include "../datatypes/user.hpp"
#include "../extras/utils.hpp"
#include "../extras/serialize.hpp"

#include "../engine/commands.hpp"

#include "../errors.hpp"

namespace epidb {
  namespace command {

    class AddProjectCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::PROJECTS, "Inserts a new project with the given parameters.");
      }

      static Parameters parameters_()
      {
        Parameter p[] = {
          Parameter("name", serialize::STRING, "projectname"),
          Parameter("description", serialize::STRING, "description of the project"),
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 3);
        return params;
      }

      static Parameters results_()
      {
        Parameter p[] = {
          Parameter("id", serialize::STRING, "id of the newly inserted project")
        };
        Parameters results(&p[0], &p[0] + 1);
        return results;
      }

    public:
      AddProjectCommand() : Command("add_project", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        // TODO: Check user
        const std::string name = parameters[0]->as_string();
        const std::string description = parameters[1]->as_string();
        const std::string user_key = parameters[2]->as_string();

        std::string msg;
        datatypes::User user;

        if (!check_permissions(user_key, datatypes::INCLUDE_EXPERIMENTS, user, msg )) {
          result.add_error(msg);
          return false;
        }

        const std::string norm_name = utils::normalize_name(name);
        if (!dba::is_project_valid(name, norm_name, msg)) {
          result.add_error(msg);
          return false;
        }

        const std::string norm_description = utils::normalize_name(description);

        std::string project_id;
        bool ret = datatypes::projects::add_project(name, norm_name, description, norm_description, user.get_id_name(),
                   project_id, msg);
        if (!ret) {
          result.add_error(msg);
        }

        // Include user in its own project
        if (!datatypes::projects::add_user_to_project(user.get_id(), project_id, true, msg)) {
          result.add_error(msg);
          return false;
        }

        // All projects are private by default
        if (!datatypes::projects::set_public(project_id, false, msg)) {
          result.add_error(msg);
          return false;
        }

        result.add_string(project_id);

        return ret;
      }
    } addProjectCommand;
  }
}
