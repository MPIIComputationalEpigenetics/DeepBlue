//
//  create_column_type_category.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 03.02.14.
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

#include "../dba/dba.hpp"
#include "../dba/column_types.hpp"

#include "../datatypes/user.hpp"

#include "../extras/utils.hpp"
#include "../extras/serialize.hpp"

#include "../engine/commands.hpp"

#include "../errors.hpp"

namespace epidb {
  namespace command {

    class CreateColumnTypeCategory: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::COLUMN_TYPES, "Create a categoric column type in DeepBlue from a set of items. As example, the STRAND column is a category column that contain the items: '+', '-', and '.' .");
      }

      static Parameters parameters_()
      {
        Parameter p[] = {
          Parameter("name", serialize::STRING, "column type name"),
          Parameter("description", serialize::STRING, "description of the column type"),
          Parameter("items", serialize::STRING, "items that are accepted for this category set", true),
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 4);
        return params;
      }

      static Parameters results_()
      {
        Parameter p[] = {
          Parameter("id", serialize::STRING, "id of the newly created column type")
        };
        Parameters results(&p[0], &p[0] + 1);
        return results;
      }

    public:
      CreateColumnTypeCategory() : Command("create_column_type_category", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        std::vector<serialize::ParameterPtr> items;

        const std::string name = parameters[0]->as_string();
        const std::string description = parameters[1]->as_string();
        const std::string user_key = parameters[3]->as_string();

        parameters[2]->children(items);
        std::vector<std::string> items_s;
        for (std::vector<serialize::ParameterPtr>::iterator it = items.begin(); it != items.end(); ++it) {
          items_s.push_back((**it).as_string());
        }

        std::string msg;
        datatypes::User user;
        if (!check_permissions(user_key, datatypes::INCLUDE_ANNOTATIONS, user, msg )) {
          result.add_error(msg);
          return false;
        }

        if (items.size() <= 1) {
          result.add_error("The category set should contains at least 2 items");
          return false;
        }

        const std::string norm_name = utils::normalize_name(name);
        if (!dba::columns::is_column_type_name_valid(name, norm_name, msg)) {
          result.add_error(msg);
          return false;
        }

        std::string norm_description = utils::normalize_name(description);

        std::string id;
        bool ret = dba::columns::create_column_type_category(user, name, norm_name, description, norm_description, items_s, id, msg);

        if (!ret) {
          result.add_error(msg);
        } else {
          result.add_string(id);
        }
        return ret;
      }
    } createColumnTypeCategory;
  }
}
