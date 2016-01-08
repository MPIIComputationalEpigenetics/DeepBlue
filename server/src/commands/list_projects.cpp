//
//  list_projects.cpp
//  epidb
//
//  Created by Felipe Albrecht on 17.06.13.
//  Copyright (c) 2016 Max Planck Institute for Informatics. All rights reserved.
//

#include "../engine/commands.hpp"

#include "../dba/dba.hpp"
#include "../dba/list.hpp"

#include "../datatypes/user.hpp"

#include "../extras/utils.hpp"
#include "../extras/serialize.hpp"

#include "../errors.hpp"

namespace epidb {
  namespace command {

    class ListProjectsCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::PROJECTS, "Lists all existing projects.");
      }

      static Parameters parameters_()
      {
        Parameter p[] = {
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 1);
        return params;
      }

      static Parameters results_()
      {
        Parameter p[] = {
          Parameter("projects", serialize::LIST, "project names")
        };
        Parameters results(&p[0], &p[0] + 1);
        return results;
      }

    public:
      ListProjectsCommand() : Command("list_projects", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        const std::string user_key = parameters[0]->as_string();

        std::string msg;
        datatypes::User user;

        if (!check_permissions(user_key, datatypes::LIST_COLLECTIONS, user, msg )) {
          result.add_error(msg);
          return false;
        }

        std::vector<utils::IdName> names;
        bool ret = dba::list::projects(user_key, names, msg);

        if (!ret) {
          result.add_error(msg);
          return false;
        }

        set_id_names_return(names, result);

        return true;
      }
    } listProjectsCommand;
  }
}
