//
//  select_gene_expressions.cpp
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
#include "../dba/queries.hpp"

#include "../datatypes/user.hpp"

#include "../engine/commands.hpp"

#include "../extras/serialize.hpp"

#include "../errors.hpp"

namespace epidb {
  namespace command {

    class SelectGeneExpressionsCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::GENES, "Select genes (by their name or ID) as genomic regions from the specified gene model.");
      }

      static Parameters parameters_()
      {
        Parameter p[] = {
          Parameter("sample_ids", serialize::STRING, "genes(s) - ENSB ID or ENSB name. Use the regular expression '.*' for selecting all." , true),
          Parameter("replicas", serialize::INTEGER, "replica(s)", true),
          Parameter("genes", serialize::STRING, "genes(s)", true),
          Parameter("gene_model", serialize::STRING, "gene model name"),
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 5);
        return params;
      }

      static Parameters results_()
      {
        Parameter p[] = {
          Parameter("id", serialize::STRING, "query id")
        };
        Parameters results(&p[0], &p[0] + 1);
        return results;
      }

    public:
      SelectGeneExpressionsCommand() : Command("select_gene_expressions", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        std::vector<serialize::ParameterPtr> sample_ids;
        std::vector<serialize::ParameterPtr> replicas;
        std::vector<serialize::ParameterPtr> genes;

        parameters[0]->children(sample_ids);
        parameters[1]->children(replicas);
        parameters[2]->children(replicas);

        const std::string gene_model = parameters[3]->as_string();

        const std::string user_key = parameters[4]->as_string();

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

        mongo::BSONObjBuilder args_builder;

        args_builder.append("sample_ids", utils::build_array(sample_ids));
        args_builder.append("replicas", utils::build_array_long(replicas));
        if (!genes.empty()) {
          args_builder.append("genes", utils::build_array_long(genes));
        }
        args_builder.append("gene_model", utils::normalize_name(gene_model));

        std::string query_id;
        if (!dba::query::store_query("gene_expressions_select", args_builder.obj(), user_key, query_id, msg)) {
          result.add_error(msg);
          return false;
        }

        result.add_string(query_id);
        return true;
      }
    } selectGeneExpressionCommand;
  }
}
