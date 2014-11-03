//
//  create_column_type_category.cpp
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

#include "../errors.hpp"

namespace epidb {
  namespace command {

    class CreateColumnTypeCategory: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::COLUMN_TYPES, "Create a column type from a category set.");
      }

      static Parameters parameters_()
      {
        Parameter p[] = {
          Parameter("name", serialize::STRING, "column type name"),
          Parameter("description", serialize::STRING, "description of the column type"),
          Parameter("default_value", serialize::STRING, "value used when the column value is missing"),
          Parameter("items", serialize::STRING, "items that are accepted for this category set", true),
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 5);
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
      CreateColumnTypeCategory() : Command("create_column_type_category", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        std::vector<serialize::ParameterPtr> items;

        const std::string name = parameters[0]->as_string();
        const std::string description = parameters[1]->as_string();
        const std::string default_value = parameters[2]->as_string();
        const std::string user_key = parameters[4]->as_string();

        parameters[3]->children(items);
        std::vector<std::string> items_s;
        for (std::vector<serialize::ParameterPtr>::iterator it = items.begin(); it != items.end(); ++it) {
          items_s.push_back((**it).as_string());
        }

        std::string msg;
        if (!Command::checks(user_key, msg)) {
          result.add_error(msg);
          return false;
        }

        if (items.size() <= 1) {
          result.add_error("The category set should contains at least 2 items");
          return false;
        }

        const std::string norm_name = utils::normalize_name(name);
        if (!dba::columns::is_column_type_name_valid(name, norm_name, msg)) {
          result.add_error(msg);
          return false;
        }

        std::string norm_description = utils::normalize_name(description);

        std::string id;
        bool ret = dba::columns::create_column_type_category(name, norm_name, description, norm_description,
                   default_value, items_s, user_key, id, msg);

        if (!ret) {
          result.add_error(msg);
        } else {
          result.add_string(id);
        }
        return ret;
      }
    } createColumnTypeCategory;
  }
}
