//
//  list_in_use.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 12.10.14.
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

#include "../dba/collections.hpp"
#include "../dba/dba.hpp"
#include "../dba/list.hpp"

#include "../datatypes/user.hpp"

#include "../extras/utils.hpp"
#include "../extras/serialize.hpp"

#include "../errors.hpp"

namespace epidb {
  namespace command {

    class ListInUseCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::GENERAL_INFORMATION, "List all terms used by the Experiments mandatory metadata that have at least one Experiment or Annotation using them.");
      }

      static Parameters parameters_()
      {
        return {
          Parameter("controlled_vocabulary", serialize::STRING, "controlled vocabulary name"),
          parameters::UserKey
        };
      }

      static Parameters results_()
      {
        return {
          Parameter("terms", serialize::LIST, "controlled_vocabulary terms with count")
        };
      }

    public:
      ListInUseCommand() : Command("list_in_use", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        const std::string controlled_vocabulary = parameters[0]->as_string();
        const std::string user_key = parameters[1]->as_string();

        std::string msg;
        datatypes::User user;

        if (!check_permissions(user_key, datatypes::LIST_COLLECTIONS, user, msg )) {
          result.add_error(msg);
          return false;
        }

        std::string collection_key;
        if (!dba::list::get_controlled_vocabulary_key(controlled_vocabulary, collection_key, msg)) {
          result.add_error(msg);
          return false;
        }

        std::vector<utils::IdNameCount> names;
        if (!dba::list::in_use(user, controlled_vocabulary, collection_key, names, msg)) {
          result.add_error(msg);
          return false;
        }

        set_id_names_count_return(names, result);

        return true;
      }
    } listInUseCommand;
  }
}
