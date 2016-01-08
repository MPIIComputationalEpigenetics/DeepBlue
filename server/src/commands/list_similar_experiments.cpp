//
//  list_similar_experiments.cpp
//  epidb
//
//  Created by Felipe Albrecht on 27.06.13.
//  Copyright (c) 2016 Max Planck Institute for Informatics. All rights reserved.
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
        return CommandDescription(categories::EXPERIMENTS, "Lists all experiments similar to the one provided.");
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
