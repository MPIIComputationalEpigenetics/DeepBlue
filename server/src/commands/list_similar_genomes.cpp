//
//  get_intersections.cpp
//  epidb
//
//  Created by Fabian Reinartz on 11.09.13.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
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

    class ListSimiliarGenomesCommand : public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::GENOMES, "Lists all genomes similar to the one provided.");
      }

      static Parameters parameters_()
      {
        Parameter p[] = {
          Parameter("name", serialize::STRING, "genome name"),
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 2);
        return params;
      }

      static Parameters results_()
      {
        Parameter p[] = {
          Parameter("genomes", serialize::LIST, "similar genome names")
        };
        Parameters results(&p[0], &p[0] + 1);
        return results;
      }

    public:
      ListSimiliarGenomesCommand() : Command("list_similar_genomes", parameters_(), results_(), desc_()) {}

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

        std::vector<utils::IdName> names;
        if (!dba::list::similar_genomes(name, user_key, names, msg)) {
          result.add_error(msg);
          return false;
        }

        set_id_names_return(names, result);

        return true;
      }

    } listSimilarGenomesCommand;
  }
}



