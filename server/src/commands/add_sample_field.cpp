//
//  add_sample_field.cpp
//  epidb
//
//  Created by Felipe Albrecht on 01.10.13.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include "../dba/dba.hpp"
#include "../extras/utils.hpp"
#include "../extras/serialize.hpp"

#include "../engine/commands.hpp"

#include "../errors.hpp"

namespace epidb {
  namespace command {

    class AddSampleFieldCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::SAMPLES, "Inserts a new sample field with the given parameters.");
      }

      static Parameters parameters_()
      {
        Parameter p[] = {
          Parameter("name", serialize::STRING, "sample field name"),
          Parameter("type", serialize::STRING, "type of the sample field: (string, numeral)"),
          Parameter("description", serialize::STRING, "description of the sample field"),
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 4);
        return params;
      }

      static Parameters results_()
      {
        Parameter p[] = {
          Parameter("id", serialize::STRING, "id of the newly inserted sample field")
        };
        Parameters results(&p[0], &p[0] + 1);
        return results;
      }

    public:
      AddSampleFieldCommand() : Command("add_sample_field", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        const std::string name = parameters[0]->as_string(); // req
        const std::string type = parameters[1]->as_string(); // req
        const std::string description = parameters[2]->as_string(); // req
        const std::string user_key = parameters[3]->as_string(); // req

        std::string msg;
        if (!Command::checks(user_key, msg)) {
          result.add_error(msg);
          return false;
        }

        const std::string norm_name = utils::normalize_name(name);
        if (!dba::is_valid_sample_field_name(name, norm_name, msg)) {
          result.add_error(msg);
          return false;
        }

        std::string norm_description = utils::normalize_name(description);

        std::string id;
        bool ret = dba::add_sample_field(name, norm_name, type, description, norm_description, user_key, id, msg);

        if (!ret) {
          result.add_error(msg);
        } else {
          result.add_string(id);
        }

        return ret;
      }
    } addSampleFieldCommand;
  }
}
