//
//  create_column_type_range.cpp
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

#include "../engine/commands.hpp"

#include "../errors.hpp"

#include "../extras/utils.hpp"
#include "../extras/serialize.hpp"

namespace epidb {
  namespace command {

    class CreateColumnTypeRange: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::COLUMN_TYPES, "Create a range column type in DeepBlue.");
      }

      static Parameters parameters_()
      {
        Parameter p[] = {
          Parameter("name", serialize::STRING, "column type name"),
          Parameter("description", serialize::STRING, "description of the column type"),
          Parameter("minimum", serialize::DOUBLE, "minimum value for this range (inclusive)"),
          Parameter("maximum", serialize::DOUBLE, "maximum value for this range (inclusive)"),
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 5);
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
      CreateColumnTypeRange() : Command("create_column_type_range", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        const std::string name = parameters[0]->as_string();
        const std::string description = parameters[1]->as_string();
        const double minimum = parameters[2]->as_double();
        const double maximum = parameters[3]->as_double();
        const std::string user_key = parameters[4]->as_string();

        std::string msg;
        datatypes::User user;

        if (!check_permissions(user_key, datatypes::INCLUDE_ANNOTATIONS, user, msg )) {
          result.add_error(msg);
          return false;
        }

        if (minimum >= maximum) {
          msg = "the maximum value should be bigger than the minimum value";
          return false;
        }

        const std::string norm_name = utils::normalize_name(name);
        if (!dba::columns::is_column_type_name_valid(name, norm_name, msg)) {
          result.add_error(msg);
          return false;
        }

        std::string norm_description = utils::normalize_name(description);

        std::string id;
        bool ret = dba::columns::create_column_type_range(name, norm_name, description, norm_description, minimum, maximum, user_key, id, msg);

        if (!ret) {
          result.add_error(msg);
        } else {
          result.add_string(id);
        }
        return ret;
      }
    } createColumnTypeRange;
  }
}

