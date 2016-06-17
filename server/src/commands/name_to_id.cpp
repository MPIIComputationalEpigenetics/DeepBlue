//
//  name_to_id.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 08.04.2016.
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

#include <map>
#include <sstream>
#include <iostream>

#include <mongo/bson/bson.h>

#include "../datatypes/user.hpp"
#include "../datatypes/metadata.hpp"

#include "../dba/dba.hpp"
#include "../dba/users.hpp"

#include "../engine/commands.hpp"

#include "../extras/serialize.hpp"
#include "../extras/utils.hpp"

#include "../errors.hpp"

namespace epidb {
  namespace command {

    class NameToIdCommand : public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::GENERAL_INFORMATION, "The ID for the given name(s).");
      }

      static  Parameters parameters_()
      {
        Parameter p[] = {
          Parameter("name", serialize::STRING, "ID or an array of IDs", true),
          Parameter("collection", serialize::STRING, "ID or an array of IDs", true),
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 3);
        return params;
      }

      static Parameters results_()
      {
        Parameter p[] = {
          Parameter("information", serialize::LIST, "List of IDs.", true)
        };
        Parameters results(&p[0], &p[0] + 1);
        return results;
      }

    public:
      NameToIdCommand() : Command("name_to_id", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        /*
        const std::string collection = parameters[0]->as_string();
        const std::string user_key = parameters[2]->as_string();

        std::string msg;
        datatypes::User user;

        std::string err_msg;
        bool has_list_collections_permissions = false;
        if (check_permissions(user_key, datatypes::LIST_COLLECTIONS, user, msg )) {
          has_list_collections_permissions = true;
        } else {
          err_msg = msg;
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

        std::vector<serialize::ParameterPtr> names_param;
        parameters[0]->children(names_param);

        if (names_param.empty()) {
          result.set_as_array(true);
          return true;
        }

        const std::string& collection_name = dba::helpers::collection_name(utils::normalize_name(collection));

        for (const auto & name_param : names_param) {
          std::string normalize_name = utils::normalize_name(names_param->as_string());

          bool ok;

          helpers::get_one()



          if (!ok) {
            result.add_error(msg);
            return false;
          }

          result.add_string(id);
        }
  */
        return true;
      }

    } nameToIdCommand;
  }
}
