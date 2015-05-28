//
//  set_project_public.cpp
//  epidb
//
//  Created by Felipe Albrecht on 27.05.15.
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
        if (!Command::checks(user_key, msg)) {
          result.add_error(msg);
          return false;
        }

        std::string id;
        if (!datatypes::projects::get_id(project, id, msg)) {
          result.add_error(msg);
          return false;
        }

        std::cerr << "id: " << id << std::endl;

        datatypes::Metadata res;
        if (!dba::info::get_project(id, res, msg)) {
          result.add_error(msg);
          return false;
        }
        std::string owner = res["user"];

        utils::IdName user;
        if (!dba::users::get_user(user_key, user, msg)) {
          result.add_error(msg);
          return false;
        }

        bool is_admin_key;
        if (!dba::users::is_admin_key(user_key, is_admin_key, msg)) {
          result.add_error(msg);
          return false;
        }

        if ((!is_admin_key) && (user.name != owner)) {
          result.add_error("User is not the project owner neither the administration");
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
