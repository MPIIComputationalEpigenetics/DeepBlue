//
//  create_column_type_calculated.cpp
//  epidb
//
//  Created by Felipe Albrecht on 28.10.14.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include "../dba/dba.hpp"
#include "../dba/column_types.hpp"
#include "../extras/utils.hpp"
#include "../extras/serialize.hpp"

#include "../engine/commands.hpp"

#include "../errors.hpp"

namespace epidb {
  namespace command {

    class CreateColumnTypeCalculated: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::COLUMN_TYPES, "Create a calculated column");
      }

      static Parameters parameters_()
      {
        Parameter p[] = {
          Parameter("name", serialize::STRING, "column type name"),
          Parameter("description", serialize::STRING, "description of the column type"),
          Parameter("code", serialize::STRING, "Lua code that will be executed"),
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 4);
        return params;
      }

      static Parameters results_()
      {
        Parameter p[] = {
          Parameter("id", serialize::STRING, "id of the newly created column type")
        };
        Parameters results(&p[0], &p[0] + 1);
        return results;
      }

    public:
      CreateColumnTypeCalculated() : Command("create_column_type_calculated", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        std::vector<serialize::ParameterPtr> items;

        const std::string name = parameters[0]->as_string();
        const std::string description = parameters[1]->as_string();
        const std::string code = parameters[2]->as_string();
        const std::string user_key = parameters[3]->as_string();

        bool ok;
        std::string msg;
        if (!dba::check_user(user_key, ok, msg)) {
          result.add_error(msg);
          return false;
        }
        if (!ok) {
          std::string s = Error::m(ERR_INVALID_USER_KEY);
          result.add_error(s);
          return false;
        }

        const std::string norm_name = utils::normalize_name(name);
        if (!dba::columns::is_column_type_name_valid(name, norm_name, msg)) {
          result.add_error(msg);
          return false;
        }

        std::string norm_description = utils::normalize_name(description);

        std::string id;
        bool ret = dba::columns::create_column_type_calculated(name, norm_name, description, norm_description,
                   code, user_key, id, msg);

        if (!ret) {
          result.add_error(msg);
        } else {
          result.add_string(id);
        }
        return ret;
      }
    } createColumnTypeCalculated;
  }
}
