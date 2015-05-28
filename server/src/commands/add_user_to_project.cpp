//
//  add_user_to_project.cpp
//  epidb
//
//  Created by Felipe Albrecht on 28.05.15.
//  Copyright (c) 2013,2014,2015 Max Planck Institute for Computer Science. All rights reserved.
//

#include "../datatypes/metadata.hpp"
#include "../datatypes/projects.hpp"

#include "../dba/dba.hpp"
#include "../dba/info.hpp"
#include "../dba/users.hpp"

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
        const std::string user = parameters[0]->as_string();
        const std::string project = parameters[1]->as_string();
        const bool include = parameters[2]->as_boolean();
        const std::string user_key = parameters[3]->as_string();

        std::string msg;
        if (!Command::checks(user_key, msg)) {
          result.add_error(msg);
          return false;
        }

        std::string project_id;
        if (!datatypes::projects::get_id(project, project_id, msg)) {
          result.add_error(msg);
          return false;
        }

        datatypes::Metadata project_res;
        if (!dba::info::get_project(project_id, project_res, msg)) {
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
        if (!dba::users::get_id(user, user_id, msg)) {
          result.add_error(msg);
          return false;
        }


        // Getting the command operator
        utils::IdName working_user;
        if (!dba::users::get_user(user_key, working_user, msg)) {
          result.add_error(msg);
          return false;
        }

        bool is_admin_key;
        if (!dba::users::is_admin_key(user_key, is_admin_key, msg)) {
          result.add_error(msg);
          return false;
        }

        // Is the command operator admin or project owner ?
        if ((!is_admin_key) && (working_user.id != owner_id)) {
          result.add_error(Error::m(ERR_PROJECT_PERMISSION, project.c_str()));
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
