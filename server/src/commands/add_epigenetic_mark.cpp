//
//  add_epigenetic_mark.cpp
//  epidb
//
//  Created by Felipe Albrecht on 16.06.13.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include "../dba/dba.hpp"
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
        return CommandDescription(categories::EPIGENETIC_MARKS, "Inserts a new epigenetic mark with the given parameters.");
      }

      static Parameters parameters_()
      {
        Parameter p[] = {
          Parameter("name", serialize::STRING, "name of the epigenetic mark"),
          Parameter("description", serialize::STRING, "description of the epigenetic mark"),
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 3);
        return params;
      }

      static Parameters results_()
      {
        Parameter p[] = {
          Parameter("id", serialize::STRING, "id of the newly inserted epigenetic mark")
        };
        Parameters results(&p[0], &p[0] + 1);
        return results;
      }

    public:
      AddEpigeneticMarkCommand() : Command("add_epigenetic_mark", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        // TODO: Check user
        const std::string name = parameters[0]->as_string();
        const std::string description = parameters[1]->as_string();
        const std::string user_key = parameters[2]->as_string();

        std::string msg;
        if (!Command::checks(user_key, msg)) {
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
        bool ret = dba::add_epigenetic_mark(name, norm_name, description, norm_description, user_key, id, msg);

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
