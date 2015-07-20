//
//  pattern.cpp
//  epidb
//
//  Created by Felipe Albrecht on 06.05.14.
//  Copyright (c) 2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include <string>
#include <sstream>

#include "../dba/dba.hpp"
#include "../datatypes/user.hpp"

#include "../engine/commands.hpp"

#include "../extras/utils.hpp"

#include "../errors.hpp"

namespace epidb {
  namespace command {

    class FindPatternCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::ANNOTATIONS, "Process an annotation that will contain all genomic positions from the given pattern.");
      }

      static Parameters parameters_()
      {
        Parameter p[] = {
          Parameter("pattern", serialize::STRING, "pattern (regular expression)"),
          parameters::Genome,
          Parameter("overlap", serialize::BOOLEAN, "if the matching should do overlap search"),
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 4);
        return params;
      }

      static Parameters results_()
      {
        Parameter p[] = {
          Parameter("id", serialize::STRING, "id of the annotation that contains the positions of the given pattern")
        };
        Parameters results(&p[0], &p[0] + 1);
        return results;
      }

    public:
      FindPatternCommand() : Command("find_pattern", parameters_(), results_(), desc_()) {}

      // TODO: Check user
      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        const std::string pattern = parameters[0]->as_string();
        const std::string genome = parameters[1]->as_string();
        bool overlap = parameters[2]->as_boolean();
        const std::string user_key = parameters[3]->as_string();

        std::string msg;
        datatypes::User user;

        if (!check_permissions(user_key, datatypes::INCLUDE_ANNOTATIONS, user, msg )) {
          result.add_error(msg);
          return false;
        }

        // TODO: check if pattern already exists

        std::string id;
        bool ret =  dba::process_pattern(genome, pattern, overlap, user_key, ip, id, msg);

        if (ret) {
          result.add_string(id);
        } else {
          result.add_error(msg);
        }
        return ret;
      }

    } findPatternCommand;

  }
}
