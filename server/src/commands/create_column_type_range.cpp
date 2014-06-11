//
//  create_column_type_range.cpp
//  epidb
//
//  Created by Felipe Albrecht on 03.02.14.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include "../dba/dba.hpp"
#include "../dba/column_types.hpp"
#include "../extras/utils.hpp"
#include "../extras/serialize.hpp"

#include "../engine/commands.hpp"

namespace epidb {
    namespace command {

      class CreateColumnTypeRange: public Command {

      private:
        static CommandDescription desc_() {
          return CommandDescription(categories::COLUMN_TYPES, "Create a column type from a category set.");
        }

        static Parameters parameters_() {
          Parameter p[] = {
            Parameter("name", serialize::STRING, "column type name"),
            Parameter("description", serialize::STRING, "description of the column type"),
            Parameter("ignore_if", serialize::STRING, "value to use for ignoring this column"),
            Parameter("minimum", serialize::DOUBLE, "minimum value for this range (inclusive)"),
            Parameter("maximum", serialize::DOUBLE, "maximum value for this range (inclusive)"),
            parameters::UserKey
          };
          Parameters params(&p[0], &p[0]+6);
          return params;
        }

        static Parameters results_() {
          Parameter p[] = {
            Parameter("id", serialize::STRING, "id of the newly created column type")
          };
          Parameters results(&p[0], &p[0]+1);
          return results;
        }

      public:
        CreateColumnTypeRange() : Command("create_column_type_range", parameters_(), results_(), desc_()) {}

        virtual bool run(const std::string& ip,
            const serialize::Parameters& parameters, serialize::Parameters& result) const
        {
          const std::string name = parameters[0]->as_string();
          const std::string description = parameters[1]->as_string();
          const std::string ignore_if = parameters[2]->as_string();
          const double minimum = parameters[3]->as_double();
          const double maximum = parameters[4]->as_double();
          const std::string user_key = parameters[5]->as_string();

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

          if (minimum >= maximum) {
            msg = "the maximum value should be bigger than the minimum value";
            return false;
          }

          const std::string norm_name = utils::normalize_name(name);
          if (!dba::columns::is_column_type_name_valid(name, norm_name, msg)) {
            result.add_error(msg);
            return false;
          }

          std::string norm_description = utils::normalize_name(description);

          std::string id;
          bool ret = dba::columns::create_column_type_range(name, norm_name, description, norm_description, ignore_if, minimum, maximum, user_key, id, msg);

          if (!ret) {
            result.add_error(msg);
          } else {
            result.add_string(id);
          }
          return ret;
        }
      } createColumnTypeRange;
  }
}

