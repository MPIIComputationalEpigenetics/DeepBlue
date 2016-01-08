//
//  get_state.cpp
//  epidb
//
//  Created by Natalie Wirth on 18.03.15.
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

#include <boost/algorithm/string/join.hpp>

#include "../datatypes/user.hpp"

#include "../dba/collections.hpp"
#include "../dba/dba.hpp"
#include "../dba/helpers.hpp"

#include "../engine/commands.hpp"

#include "../errors.hpp"

namespace epidb {
  namespace command {

    class GetStateCommand : public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::GENERAL_INFORMATION, "Returns the current state of specific data.");
      }

      static Parameters parameters_()
      {
        Parameter p[] = {
          Parameter("data_name", serialize::STRING, "Name of the data to lookup the state for"),
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 2);
        return params;
      }

      static Parameters results_()
      {
        Parameter p[] = {
          Parameter("data_state", serialize::INTEGER, "State of the data")
        };
        Parameters results(&p[0], &p[0] + 1);
        return results;
      }

      const std::vector<std::string> allowed_names {dba::Collections::ANNOTATIONS(),
              dba::Collections::BIOSOURCES(), dba::Collections::COLUMN_TYPES(),
              dba::Collections::EXPERIMENTS(), dba::Collections::EPIGENETIC_MARKS(),
              dba::Collections::GENOMES(), dba::Collections::PROJECTS(),
              dba::Collections::SAMPLES(), dba::Collections::TECHNIQUES()
      };

      static std::string collection_to_operation(std::string counter_name)
      {
        return counter_name + "_operations";
      }

    public:
      GetStateCommand() : Command("get_state", parameters_(), results_(), desc_())
      {
      }

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

        if (std::find(allowed_names.begin(), allowed_names.end(), name) == allowed_names.end()) {
          result.add_error(name + " is not an allowed data name. Allowed data names are: " + boost::algorithm::join(allowed_names, ", "));
          return false;
        }

        int counter = 0;
        if (!dba::helpers::get_counter(collection_to_operation(name), counter, msg)) {
          return false;
        }
        result.add_int(counter);

        return true;
      }
    } getStateCommand;
  }
}
