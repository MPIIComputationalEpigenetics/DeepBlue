//
//  count_gene_ontology_terms.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 01.03.17.
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

#include <future>
#include <string>
#include <thread>
#include <tuple>

#include "../engine/commands.hpp"

#include "../dba/gene_ontology.hpp"

#include "../datatypes/user.hpp"

#include "../extras/utils.hpp"
#include "../extras/serialize.hpp"

#include "../errors.hpp"

namespace epidb {
  namespace command {

    class CountGeneOntologyTermsCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::EXPERIMENTS, "Summarize the controlled_vocabulary fields, from experiments that match the selection criteria. It is similar to the 'collection_experiments_count' command, but this command return the summarization for all controlled_vocabulary terms.");
      }

      static Parameters parameters_()
      {
        return {
          parameters::Genes,
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
          Parameter("faceting", serialize::MAP, "Map with the mandatory fields of the experiments metadata, where each contains a list of terms that appears.")
        };
      }

    public:
      CountGeneOntologyTermsCommand() : Command("count_gene_ontology_terms", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        std::vector<serialize::ParameterPtr> genes;
        parameters[0]->children(genes);

        const std::string gene_model = parameters[1]->as_string();
        std::string norm_gene_model = utils::normalize_name(gene_model);

        std::vector<serialize::ParameterPtr> chromosomes;
        parameters[2]->children(chromosomes);

        const int start = parameters[3]->isNull() ? -1 : parameters[3]->as_long();
        const int end = parameters[4]->isNull() ? -1 : parameters[4]->as_long();

        const std::string user_key = parameters[5]->as_string();

        std::string msg;
        datatypes::User user;

        if (!check_permissions(user_key, datatypes::LIST_COLLECTIONS, user, msg )) {
          result.add_error(msg);
          return false;
        }

        std::vector<std::string> gene_names = utils::build_vector(genes);
        std::vector<std::string> chromosome_names = utils::build_vector(chromosomes);
        std::vector<utils::IdNameCount> counts;
        size_t total_go_terms;
        if (!dba::gene_ontology::count_go_terms_in_genes(chromosome_names, start, end,
            "",  gene_names, gene_model, norm_gene_model,
            counts, total_go_terms, msg)) {
          result.add_error(msg);
          return false;
        }

        set_id_names_count_return(counts, result);

        return true;
      }
    } countGeneOntologyTermsCommand;
  }
}
