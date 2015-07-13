//
//  add_technique.cpp
//  epidb
//
//  Created by Felipe Albrecht on 10.02.14.
//  Copyright (c) 2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include "../dba/dba.hpp"
#include "../datatypes/user.hpp"
#include "../entities/users.hpp"
#include "../extras/utils.hpp"
#include "../extras/serialize.hpp"

#include "../engine/commands.hpp"

#include "../errors.hpp"

namespace epidb {
  namespace command {

    class AddTechniqueCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::TECHNIQUES, "Inserts a technique with the given parameters.");
      }

      static Parameters parameters_()
      {
        Parameter p[] = {
          Parameter("name", serialize::STRING, "technique name"),
          Parameter("description", serialize::STRING, "description of technique"),
          Parameter("extra_metadata", serialize::MAP, "additional metadata"),
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 4);
        return params;
      }

      static Parameters results_()
      {
        Parameter p[] = {
          Parameter("id", serialize::STRING, "id of the newly inserted technique")
        };
        Parameters results(&p[0], &p[0] + 1);
        return results;
      }

    public:
      AddTechniqueCommand() : Command("add_technique", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        const std::string name = parameters[0]->as_string();
        const std::string description = parameters[1]->as_string();

        const std::string user_key = parameters[3]->as_string();

        std::string msg;

        datatypes::User user;
        if (!dba::get_user_by_key(user_key, user, msg)) {
          result.add_error(msg);
          return false;
        }

        if (!user.has_permission(datatypes::INCLUDE_COLLECTION_TERMS)) {
          result.add_error(Error::m(ERR_INSUFFICIENT_PERMISSION));
          return false;
        }

        datatypes::Metadata extra_metadata;
        if (!read_metadata(parameters[2], extra_metadata, msg)) {
          result.add_error(msg);
          return false;
        }

        std::string norm_name = utils::normalize_name(name);
        if (!dba::is_valid_technique_name(name, norm_name, msg)) {
          result.add_error(msg);
          return false;
        }

        std::string norm_description = utils::normalize_name(description);

        std::string id;
        bool ret = dba::add_technique(name, norm_name, description, norm_description, extra_metadata, user_key, id, msg);
        if (!ret) {
          result.add_error(msg);
        } else {
          result.add_string(id);
        }
        return ret;
      }
    } addTechniqueCommand;
  }
}
