//
//  change_extra_metadata.cpp
//  DeepBlue Epigenomic Data Server

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
//  Created by Felipe Albrecht on 29.09.2014.
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

#include <string>

#include "../dba/changes.hpp"
#include "../dba/dba.hpp"
#include "../dba/users.hpp"
#include "../engine/commands.hpp"
#include "../extras/serialize.hpp"

namespace epidb {
  namespace command {

    class ChangeExtraMetadataCommand : public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::DATA_MODIFICATION,
                                  "Change the extra metadata content for experiments, annotations, biosources, and samples.");
      }

      static  Parameters parameters_()
      {
        Parameter p[] = {
          Parameter("id", serialize::STRING, "id of the data"),
          Parameter("extra_metadata_key", serialize::STRING, "extra_metadata key"),
          Parameter("extra_metadata_value", serialize::STRING, "extra_metadata key (empty for delete this key)"),
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 4);
        return params;
      }

      static Parameters results_()
      {
        Parameter p[] = {
          Parameter("id", serialize::STRING, "id of the modified data")
        };
        Parameters results(&p[0], &p[0] + 1);
        return results;
      }

    public:
      ChangeExtraMetadataCommand() : Command("change_extra_metadata", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        const std::string id = parameters[0]->as_string();
        const std::string extra_metadata_key = parameters[1]->as_string();
        const std::string extra_metadata_value = parameters[2]->as_string();
        const std::string user_key = parameters[3]->as_string();

        std::string msg;
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

        if (!dba::changes::change_extra_metadata(id, extra_metadata_key, extra_metadata_value, msg)) {
          result.add_error(msg);
          return false;
        }

        result.add_string(id);

        return true;
      }

    } changeExtraMetadataCommand;
  }
}
