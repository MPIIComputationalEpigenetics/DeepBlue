//
//  create_experiments_set.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 07.09.16.
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

#include "../dba/experiments.hpp"
#include "../datatypes/user.hpp"
#include "../extras/utils.hpp"
#include "../extras/serialize.hpp"

#include "../engine/commands.hpp"

#include "../log.hpp"
#include "../errors.hpp"

namespace epidb {
  namespace command {

    class CreateExperimentsSetCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::BIOSOURCE_RELATIONSHIP, "Create a set of experiments to be shared among others users");
      }

      static  Parameters parameters_()
      {
        Parameter p[] = {
          Parameter("name", serialize::STRING, "experiments set name"),
          Parameter("description", serialize::STRING, "experiments set description"),
          Parameter("public", serialize::BOOLEAN, "True is others users can access this list"),
          Parameter("experiment_name", serialize::STRING, "name(s) of selected experiment(s)", true),
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 5);
        return params;
      }

      static Parameters results_()
      {
        Parameter p[] = {
          Parameter("id", serialize::STRING, "id of the experiments set")
        };
        Parameters results(&p[0], &p[0] + 1);
        return results;
      }

    public:
      CreateExperimentsSetCommand() : Command("create_experiments_set", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        const std::string name = parameters[0]->as_string();
        const std::string description = parameters[1]->as_string();
        const bool is_public = parameters[2]->as_boolean();
        const std::string user_key = parameters[4]->as_string();

        std::string msg;
        datatypes::User user;
        if (!check_permissions(user_key, datatypes::GET_DATA, user, msg )) {
          result.add_error(msg);
          return false;
        }

        std::vector<serialize::ParameterPtr> experiment_names;
        parameters[3]->children(experiment_names);
        std::vector<std::string> exp_names_string = utils::build_vector(experiment_names);

        std::string set_id;
        if (!dba::experiments::create_experiment_set(exp_names_string, name, description, is_public, set_id, msg)) {
          result.add_error(msg);
          return false;
        }

        result.add_string(set_id);

        return true;
      }

    } createExperimentsSetCommand;
  }
}
