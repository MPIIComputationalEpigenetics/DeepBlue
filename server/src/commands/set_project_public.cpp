//
//  set_project_public.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 27.05.15.
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
#include "../datatypes/user.hpp"

#include "../dba/dba.hpp"
#include "../dba/info.hpp"
#include "../dba/list.hpp"
#include "../dba/users.hpp"

#include "../engine/commands.hpp"

#include "../extras/serialize.hpp"
#include "../extras/utils.hpp"

#include "../errors.hpp"

namespace epidb {
  namespace command {

    class SetProjectPublicCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::PROJECTS, "Define a project as public. This means that all DeepBlue users can then access its data. You must be the project owner to perform this operation.");
      }

      static  Parameters parameters_()
      {
        Parameter p[] = {
          Parameter("project", serialize::STRING, "Project name or ID"),
          Parameter("set", serialize::BOOLEAN, "True to set the project as public of false for unset"),
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 3);
        return params;
      }

      static Parameters results_()
      {
        Parameter p[] = {
          Parameter("id", serialize::STRING, "id of the project"),
        };
        Parameters results(&p[0], &p[0] + 1);
        return results;
      }

    public:
      SetProjectPublicCommand() : Command("set_project_public", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        const std::string project = parameters[0]->as_string();
        const bool set = parameters[1]->as_boolean();
        const std::string user_key = parameters[2]->as_string();

        std::string msg;

        std::string id;
        if (!datatypes::projects::get_id(project, id, msg)) {
          result.add_error(msg);
          return false;
        }

        datatypes::User user;
        if (!check_permissions(user_key, datatypes::ADMIN, user, msg )) {
            datatypes::User user2;
            if (!dba::users::get_owner(id, user2, msg)) {
                result.add_error(msg);
                return false;
            }
            if (user.get_id() != user2.get_id()) {
                result.add_error(msg);
                return false;
            }
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
        if (!dba::info::get_project(id, user_projects, project_res, msg)) {
          result.add_error(msg);
          return false;
        }
        std::string owner = project_res["user"];

        if (!user.is_admin() && user.get_id() != owner) {
          result.add_error(Error::m(ERR_PERMISSION_PROJECT, project));
          return false;
        }

        if (!datatypes::projects::set_public(id, set, msg)) {
          result.add_error(msg);
          return false;
        }

        result.add_string(id);
        return true;
      }

    } setProjectPublic;
  }
}
