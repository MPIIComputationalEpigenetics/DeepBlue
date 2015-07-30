//
//  set_project_public.cpp
//  epidb
//
//  Created by Felipe Albrecht on 27.05.15.
//  Copyright (c) 2013,2014,2015 Max Planck Institute for Computer Science. All rights reserved.
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
        return CommandDescription(categories::PROJECTS, "Set a project as public. You must be the project owner to perform this operation.");
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


        if (!user.is_admin() && user.get_name() != owner) {
          result.add_error(Error::m(ERR_PROJECT_PERMISSION, project.c_str()));
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
