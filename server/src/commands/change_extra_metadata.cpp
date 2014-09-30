//
//  change_extra_metadata.cpp
//  epidb
//
//  Created by Felipe Albrecht on 29.09.2013.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include <string>

#include "../dba/changes.hpp"
#include "../dba/dba.hpp"
#include "../engine/commands.hpp"
#include "../extras/serialize.hpp"

namespace epidb {
  namespace command {

    class ChangeExtraMetadataCommand : public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::DATA_MODIFICATION,
                                  "Change the extra metadata content for experiments, annotations, biosources, and samples.");
      }

      static  Parameters parameters_()
      {
        Parameter p[] = {
          Parameter("id", serialize::STRING, "id of the data"),
          Parameter("extra_metadata_key", serialize::STRING, "extra_metadata key"),
          Parameter("extra_metadata_value", serialize::STRING, "extra_metadata key (empty for delete this key)"),
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 4);
        return params;
      }

      static Parameters results_()
      {
        Parameter p[] = {
          Parameter("id", serialize::STRING, "id of the modified data")
        };
        Parameters results(&p[0], &p[0] + 1);
        return results;
      }

    public:
      ChangeExtraMetadataCommand() : Command("change_extra_metadata", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        const std::string id = parameters[0]->as_string();
        const std::string extra_metadata_key = parameters[1]->as_string();
        const std::string extra_metadata_value = parameters[2]->as_string();
        const std::string user_key = parameters[3]->as_string();

        std::string msg;
        bool ok;
        if (!dba::check_user(user_key, ok, msg)) {
          result.add_error(msg);
          return false;
        }
        if (!ok) {
          result.add_error("Invalid user key.");
          return false;
        }

        if (!dba::changes::change_extra_metadata(id, extra_metadata_key, extra_metadata_value, msg)) {
          result.add_error(msg);
          return false;
        }

        result.add_string(id);

        return true;
      }

    } changeExtraMetadataCommand;
  }
}
