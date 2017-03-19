//
//  add_epigenetic_mark.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 16.06.13.
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
#include "../datatypes/user.hpp"
#include "../extras/utils.hpp"
#include "../extras/serialize.hpp"

#include "../engine/commands.hpp"

#include "../errors.hpp"

namespace epidb {
  namespace command {

    class AddEpigeneticMarkCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::EPIGENETIC_MARKS, "Include an Epigenetic Mark such as, for instance, a specific type of histone modification, in DeepBlue.");
      }

      static Parameters parameters_()
      {
        return {
          Parameter("name", serialize::STRING, "name of the epigenetic mark"),
          Parameter("description", serialize::STRING, "description of the epigenetic mark"),
          parameters::AdditionalExtraMetadata,
          parameters::UserKey
        };
      }

      static Parameters results_()
      {
        return {
          Parameter("id", serialize::STRING, "id of the newly inserted epigenetic mark")
        };
      }

    public:
      AddEpigeneticMarkCommand() : Command("add_epigenetic_mark", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        // TODO: Check user
        const std::string name = parameters[0]->as_string();
        const std::string description = parameters[1]->as_string();
        const std::string user_key = parameters[3]->as_string();

        std::string msg;
        datatypes::User user;

        if (!check_permissions(user_key, datatypes::INCLUDE_COLLECTION_TERMS, user, msg )) {
          result.add_error(msg);
          return false;
        }

        datatypes::Metadata extra_metadata;
        if (!read_metadata(parameters[2], extra_metadata, msg)) {
          result.add_error(msg);
          return false;
        }

        std::string norm_name = utils::normalize_epigenetic_mark(name);
        if (!dba::is_valid_epigenetic_mark(name, norm_name, msg)) {
          result.add_error(msg);
          return false;
        }

        const std::string norm_description = utils::normalize_name(description);

        std::string id;
        bool ret = dba::add_epigenetic_mark(name, norm_name, description, norm_description, extra_metadata, user_key, id, msg);

        if (!ret) {
          result.add_error(msg);
        } else {
          result.add_string(id);
        }
        return ret;
      }
    } addEpigeneticMarkCommand;
  }
}
