//
//  list_projects.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 17.06.13.
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

#include "../engine/commands.hpp"

#include "../dba/dba.hpp"
#include "../dba/list.hpp"

#include "../datatypes/user.hpp"
#include "../datatypes/projects.hpp"

#include "../extras/utils.hpp"
#include "../extras/serialize.hpp"

#include "../errors.hpp"

namespace epidb {
  namespace command {

    class ListProjectsCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::PROJECTS, "List Projects included in DeepBlue.");
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

        std::vector<std::string> project_names = user.projects();
        std::vector<utils::IdName> names;

        for (const auto& p_name: project_names) {
          std::string id;
          if (!datatypes::projects::get_id(p_name, id, msg)) {
            result.add_error(msg);
            return false;
          }
          names.emplace_back(id, p_name);
        }

        set_id_names_return(names, result);

        return true;
      }
    } listProjectsCommand;
  }
}
