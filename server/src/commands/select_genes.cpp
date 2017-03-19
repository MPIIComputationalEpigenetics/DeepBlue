//
//  select_genes.cpp
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

    class SelectGenesCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::GENES, "Select genes (by their name or ID) as genomic regions from the specified gene model.");
      }

      static Parameters parameters_()
      {
        return {
          parameters::Genes,
          parameters::GeneOntologyTerms,
          parameters::GeneModel,
          Parameter("chromosome", serialize::STRING, "chromosome name(s)", true),
          Parameter("start", serialize::INTEGER, "minimum start region"),
          Parameter("end", serialize::INTEGER, "maximum end region"),
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
      SelectGenesCommand() : Command("select_genes", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        std::vector<serialize::ParameterPtr> genes;
        parameters[0]->children(genes);

        std::vector<serialize::ParameterPtr> go_terms;
        parameters[1]->children(go_terms);

        const std::string gene_model = parameters[2]->as_string();

        std::vector<serialize::ParameterPtr> chromosomes;
        parameters[3]->children(chromosomes);

        const int start = parameters[3]->isNull() ? -1 : parameters[4]->as_long();
        const int end = parameters[4]->isNull() ? -1 : parameters[5]->as_long();

        const std::string user_key = parameters[6]->as_string();

        std::string msg;
        datatypes::User user;

        if (!check_permissions(user_key, datatypes::GET_DATA, user, msg )) {
          result.add_error(msg);
          return false;
        }

        if (gene_model.empty()) {
          result.add_error(Error::m(ERR_USER_GENE_MODEL_MISSING));
          return false;
        }

        mongo::BSONObjBuilder args_builder;
        if (start > 0) {
          args_builder.append("start", (int) start);
        }
        if (end > 0) {
          args_builder.append("end", (int) end);
        }

        if (!chromosomes.empty()) {
          args_builder.append("chromosomes", utils::build_array(chromosomes));
        }

        args_builder.append("genes", utils::build_array(genes));
        args_builder.append("go_terms", utils::build_array(go_terms));
        args_builder.append("gene_model", utils::normalize_name(gene_model));

        std::string query_id;
        if (!dba::query::store_query("genes_select", args_builder.obj(), user_key, query_id, msg)) {
          result.add_error(msg);
          return false;
        }

        result.add_string(query_id);
        return true;
      }
    } selectGenesCommand;
  }
}
