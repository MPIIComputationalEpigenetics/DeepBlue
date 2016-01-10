//
//  list_similar_projects.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 27.06.13.
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

#include "../extras/utils.hpp"
#include "../extras/serialize.hpp"

#include "../errors.hpp"

namespace epidb {
  namespace command {

    class ListSimilarProjectsCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::PROJECTS, "Lists all projects similar to the one provided.");
      }

      static Parameters parameters_()
      {
        Parameter p[] = {
          Parameter("name", serialize::STRING, "project name"),
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 2);
        return params;
      }

      static Parameters results_()
      {
        Parameter p[] = {
          Parameter("projects", serialize::LIST, "similar project names")
        };
        Parameters results(&p[0], &p[0] + 1);
        return results;
      }

    public:
      ListSimilarProjectsCommand() : Command("list_similar_projects", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        const std::string name = parameters[0]->as_string();
        const std::string user_key = parameters[1]->as_string();

        std::string msg;
        datatypes::User user;

        if (!check_permissions(user_key, datatypes::LIST_COLLECTIONS, user, msg )) {
          result.add_error(msg);
          return false;
        }

        std::vector<utils::IdName> names;
        if (!dba::list::similar_projects(name, user_key, names, msg)) {
          result.add_error(msg);
          return false;
        }

        set_id_names_return(names, result);

        return true;
      }
    } listSimilarProjectsCommand;
  }
}
