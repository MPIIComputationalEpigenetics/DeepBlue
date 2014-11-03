//
//  list_annotations.cpp
//  epidb
//
//  Created by Felipe Albrecht on 11.09.13.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include "../engine/commands.hpp"

#include "../dba/dba.hpp"
#include "../dba/list.hpp"

#include "../extras/utils.hpp"
#include "../extras/serialize.hpp"

namespace epidb {
  namespace command {

    class ListAnnotationsCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::ANNOTATIONS, "Lists all existing annotations.");
      }

      static Parameters parameters_()
      {
        Parameter p[] = {
          parameters::GenomeMultiple,
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 2);
        return params;
      }

      static Parameters results_()
      {
        Parameter p[] = {
          Parameter("annotations", serialize::LIST, "annotation ids")
        };
        Parameters results(&p[0], &p[0] + 1);
        return results;
      }

    public:
      ListAnnotationsCommand() : Command("list_annotations", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        const std::string user_key = parameters[1]->as_string();

        std::vector<serialize::ParameterPtr> genomes;
        parameters[0]->children(genomes);


        std::string msg;
        if (!Command::checks(user_key, msg)) {
          result.add_error(msg);
          return false;
        }

        if (genomes.size() == 0) {
          result.add_error("Genome was not informed.");
          return false;
        }

        std::vector<utils::IdName> names;

        std::vector<serialize::ParameterPtr>::iterator it;
        for (it = genomes.begin(); it != genomes.end(); ++it) {
          std::string genome = (**it).as_string();

          std::vector<utils::IdName> res;
          if (!dba::list::annotations(genome, user_key, res, msg)) {
            result.add_error(msg);
          }
          names.insert(names.end(), res.begin(), res.end());
        }

        set_id_names_return(names, result);

        return true;
      }
    } ListAnnotationsCommand;
  }
}
