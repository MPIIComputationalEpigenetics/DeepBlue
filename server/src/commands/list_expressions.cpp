//
//  list_expressions.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 17.07.16.
//  Copyright (c) 2016 Max Planck Institute for Informatics. All rights reserved.

//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.

//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.

//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#include "../engine/commands.hpp"

#include "../dba/dba.hpp"
#include "../dba/list.hpp"

#include "../datatypes/expressions.hpp"
#include "../datatypes/user.hpp"

#include "../extras/utils.hpp"
#include "../extras/serialize.hpp"

#include "../errors.hpp"
#include "../macros.hpp"

namespace epidb {
  namespace command {

    class ListExpressionsCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::EXPRESSIONS, "List the Expression currently available in DeepBlue. A expression is a set of data with an identifier and an expression value.");
      }

      static Parameters parameters_()
      {
        Parameter p[] = {
          parameters::ExpressionType,
          Parameter("sample_id", serialize::STRING, "sample ID(s)", true),
          Parameter("replica", serialize::INTEGER, "replica(s)", true),
          Parameter("project", serialize::STRING, "project(s) name", true),
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 5);
        return params;
      }

      static Parameters results_()
      {
        Parameter p[] = {
          Parameter("expressions", serialize::LIST, "expressions names and IDS")
        };
        Parameters results(&p[0], &p[0] + 1);
        return results;
      }

    public:
      ListExpressionsCommand() : Command("list_expressions", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        std::vector<serialize::ParameterPtr> sample_ids;
        std::vector<serialize::ParameterPtr> replicas;
        std::vector<serialize::ParameterPtr> projects;

        const std::string expression_type_name = parameters[0]->as_string();
        parameters[1]->children(sample_ids);
        parameters[2]->children(replicas);
        parameters[3]->children(projects);
        const std::string user_key = parameters[4]->as_string();

        std::string msg;
        datatypes::User user;

        if (!check_permissions(user_key, datatypes::LIST_COLLECTIONS, user, msg )) {
          result.add_error(msg);
          return false;
        }

        GET_EXPRESSION_TYPE(expression_type_name, expression_type)

        mongo::BSONObj query;

        if (!expression_type->build_list_expressions_query(sample_ids, replicas, projects, user_key, query, msg)) {
          result.add_error(msg);
          return false;
        }

        std::vector<utils::IdName> names;
        if (!expression_type->list(query, names, msg)) {
          result.add_error(msg);
        }

        set_id_names_return(names, result);

        return true;
      }
    } listExpressionsCommand;
  }
}
