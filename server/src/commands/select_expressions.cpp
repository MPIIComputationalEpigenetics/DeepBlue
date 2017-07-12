//
//  select_expressions.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 10.09.15.
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

#include <sstream>
#include <map>
#include <set>

#include "../dba/dba.hpp"
#include "../dba/genomes.hpp"
#include "../dba/helpers.hpp"
#include "../dba/list.hpp"
#include "../dba/queries.hpp"

#include "../datatypes/expressions_manager.hpp"
#include "../datatypes/user.hpp"

#include "../engine/commands.hpp"

#include "../extras/serialize.hpp"

#include "../errors.hpp"
#include "../macros.hpp"

namespace epidb {
  namespace command {

    class SelectExpressionsCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::EXPRESSIONS, "Select expressions (by their name or ID) as genomic regions from the specified model.");
      }

      static Parameters parameters_()
      {
        return {
          parameters::ExpressionType,
          Parameter("sample_ids", serialize::STRING, "id(s) of selected sample(s)" , true),
          Parameter("replicas", serialize::INTEGER, "replica(s)", true),
          Parameter("identifiers", serialize::STRING, "identifier(s) (for genes: ensembl ID or ENSB name).", true),
          Parameter("projects", serialize::STRING, "projects(s)", true),
          Parameter("gene_model", serialize::STRING, "gene model name"),
          parameters::UserKey
        };
      }

      static Parameters results_()
      {
        return {
          Parameter("id", serialize::STRING, "query id")
        };
      }

    public:
      SelectExpressionsCommand() : Command("select_expressions", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        std::vector<serialize::ParameterPtr> sample_ids;
        std::vector<serialize::ParameterPtr> replicas;
        std::vector<serialize::ParameterPtr> genes;
        std::vector<serialize::ParameterPtr> projects;

        const std::string expression_type_name = parameters[0]->as_string();
        parameters[1]->children(sample_ids);
        parameters[2]->children(replicas);
        parameters[3]->children(genes);
        parameters[4]->children(projects);

        const std::string gene_model = parameters[5]->as_string();

        const std::string user_key = parameters[6]->as_string();

        std::string msg;
        datatypes::User user;

        if (!check_permissions(user_key, datatypes::GET_DATA, user, msg )) {
          result.add_error(msg);
          return false;
        }

        if (sample_ids.empty()) {
          result.add_error(Error::m(ERR_USER_SAMPLE_MISSING));
          return false;
        }

        if (gene_model.empty()) {
          result.add_error(Error::m(ERR_USER_GENE_MODEL_MISSING));
          return false;
        }

        GET_EXPRESSION_TYPE(expression_type_name, expression_type)

        mongo::BSONObjBuilder args_builder;
        args_builder.append("expression_type", expression_type_name);
        args_builder.append("sample_ids", utils::build_array(sample_ids));
        args_builder.append("replicas", utils::build_array_long(replicas));
        if (!genes.empty()) {
          args_builder.append("genes", utils::build_array(genes));
        }
        args_builder.append("gene_model", utils::normalize_name(gene_model));


        if (projects.size() > 0) {
          // Filter the projects that are available to the user
          std::vector<serialize::ParameterPtr> filtered_projects;
          for (const auto& project : projects) {
            std::string norm_project = utils::normalize_name(project->as_string());
            bool found = false;
            for (const auto& user_project : user.projects()) {
              std::string norm_user_project = utils::normalize_name(user_project);
              if (norm_project == norm_user_project) {
                filtered_projects.push_back(project);
                found = true;
                break;
              }
            }

            if (!found) {
              result.add_error(Error::m(ERR_INVALID_PROJECT, project->as_string()));
              return false;
            }
          }

          args_builder.append("project", utils::build_array(filtered_projects));
          args_builder.append("norm_project", utils::build_normalized_array(filtered_projects));
        }

        std::string query_id;
        if (!dba::query::store_query(user, "expressions_select", args_builder.obj(), query_id, msg)) {
          result.add_error(msg);
          return false;
        }

        result.add_string(query_id);
        return true;
      }
    } selectExpressionCommand;
  }
}
