//
//  list_genomes.cpp
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

    class ListGenomesCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::GENOMES, "Lists all existing genomes.");
      }

      static Parameters parameters_()
      {
        Parameter p[] = {
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 1);
        return params;
      }

      static Parameters results_()
      {
        Parameter p[] = {
          Parameter("genomes", serialize::LIST, "genome names")
        };
        Parameters results(&p[0], &p[0] + 1);
        return results;
      }

    public:
      ListGenomesCommand() : Command("list_genomes", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        // TODO: Check user
        const std::string user_key = parameters[0]->as_string();

        std::string msg;
        datatypes::User user;

        if (!check_permissions(user_key, datatypes::LIST_COLLECTIONS, user, msg )) {
          result.add_error(msg);
          return false;
        }

        std::vector<utils::IdName> names;
        bool ret = dba::list::genomes(user_key, names, msg);

        if (!ret) {
          result.add_error(msg);
          return false;
        }

        set_id_names_return(names, result);

        return true;
      }
    } listGenomesCommand;
  }
}
