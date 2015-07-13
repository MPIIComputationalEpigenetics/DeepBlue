//
//  add_project.cpp
//  epidb
//
//  Created by Felipe Albrecht on 16.06.13.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include "../datatypes/projects.hpp"

#include "../dba/dba.hpp"
#include "../dba/users.hpp"
#include "../datatypes/user.hpp"
#include "../entities/users.hpp"
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

        datatypes::User user1;
        if (!dba::get_user_by_key(user_key, user1, msg)) {
          result.add_error(msg);
          return false;
        }

        if (!user1.has_permission(datatypes::INCLUDE_EXPERIMENTS)) {
          result.add_error(Error::m(ERR_INSUFFICIENT_PERMISSION));
          return false;
        }

        utils::IdName user;
        if (!dba::users::get_user(user_key, user, msg)) {
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
        bool ret = datatypes::projects::add_project(name, norm_name, description, norm_description, user, project_id, msg);
        if (!ret) {
          result.add_error(msg);
        }

        // Include user in its own project
        if (!datatypes::projects::add_user_to_project(user.id, project_id, true, msg)) {
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
