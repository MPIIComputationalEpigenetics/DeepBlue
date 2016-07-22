//
//  list_similar_experiments.cpp
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

    class ListSimilarExperimentsCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::EXPERIMENTS, "List all Experiments that have a similar name compared to the provided name. The similarity is calculated using the Levenshtein method.");
      }

      static Parameters parameters_()
      {
        Parameter p[] = {
          Parameter("name", serialize::STRING, "experiment name"),
          parameters::GenomeMultiple,
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 3);
        return params;
      }

      static Parameters results_()
      {
        Parameter p[] = {
          Parameter("experiments", serialize::LIST, "similar experiment names")
        };
        Parameters results(&p[0], &p[0] + 1);
        return results;
      }

    public:
      ListSimilarExperimentsCommand() : Command("list_similar_experiments", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        std::string name = parameters[0]->as_string();
        const std::string user_key = parameters[2]->as_string();

        std::vector<serialize::ParameterPtr> genomes;
        parameters[1]->children(genomes);

        std::string msg;
        datatypes::User user;

        if (!check_permissions(user_key, datatypes::LIST_COLLECTIONS, user, msg )) {
          result.add_error(msg);
          return false;
        }

        if (genomes.size() == 0) {
          result.add_error(Error::m(ERR_USER_GENOME_MISSING));
          return false;
        }

        std::vector<utils::IdName> names;

        std::vector<serialize::ParameterPtr>::iterator it;
        for (it = genomes.begin(); it != genomes.end(); ++it) {
          std::string genome = (**it).as_string();

          std::vector<utils::IdName> res;
          if (!dba::list::similar_experiments(name, genome, user_key, res, msg)) {
            result.add_error(msg);
          }
          names.insert(names.end(), res.begin(), res.end());
        }

        set_id_names_return(names, result);

        return true;
      }
    } listSimilarExperimentsCommand;
  }
}
