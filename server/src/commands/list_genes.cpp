//
//  list_annotations.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 11.09.13.
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

#include "../datatypes/user.hpp"

#include "../extras/utils.hpp"
#include "../extras/serialize.hpp"

#include "../errors.hpp"

#include <ctime>

namespace epidb {
  namespace command {

    class ListGenesCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::GENES, "List all the Gene currently available in DeepBlue.");
      }

      static Parameters parameters_()
      {
        Parameter p[] = {
          Parameter("gene_id_or_name", serialize::STRING, "Name(s) or ID(s) (ensembl id) of the selected gene(s)", true),
          Parameter("chromosome", serialize::STRING, "chromosome name(s)", true),
          Parameter("start", serialize::INTEGER, "minimum start region"),
          Parameter("end", serialize::INTEGER, "maximum end region"),
          parameters::GeneModels,
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 6);
        return params;
      }

      static Parameters results_()
      {
        Parameter p[] = {
          Parameter("genes", serialize::LIST, "genes names and its content")
        };
        Parameters results(&p[0], &p[0] + 1);
        return results;
      }

    public:
      ListGenesCommand() : Command("list_genes", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        datatypes::User user;
        std::string msg;
        const std::string user_key = parameters[5]->as_string();

        if (!check_permissions(user_key, datatypes::LIST_COLLECTIONS, user, msg )) {
          result.add_error(msg);
          return false;
        }

        std::vector<serialize::ParameterPtr> gene_id_or_name;
        std::vector<serialize::ParameterPtr> chromosomes;
        std::vector<serialize::ParameterPtr> gene_models;
        parameters[0]->children(gene_id_or_name);
        parameters[1]->children(chromosomes);
        parameters[4]->children(gene_models);

        const Position start = parameters[2]->isNull() ? -1 : parameters[2]->as_long();
        const Position end = parameters[3]->isNull() ? -1 : parameters[3]->as_long();

        std::vector<std::string> norm_gene_models;

        for (auto it = gene_models.begin(); it != gene_models.end(); ++it) {
          std::string gene_model = (**it).as_string();
          norm_gene_models.emplace_back(utils::normalize_name(gene_model));
        }

          clock_t list_time = clock();
        std::vector<mongo::BSONObj> genes;
        if (!dba::list::genes(user_key, utils::build_vector(gene_id_or_name), utils::build_vector(chromosomes), start, end, norm_gene_models, genes, msg)) {
          result.add_error(msg);
        }
        std::cerr << "list in " << (( ((float)  clock()) - list_time) / CLOCKS_PER_SEC) << std::endl;

        result.set_as_array(true);
        for (auto gene: genes) {
          result.add_param(utils::bson_to_parameters(gene));
        }

        std::cerr << "total in " << (( ((float)  clock()) - list_time) / CLOCKS_PER_SEC) << std::endl;

        return true;
      }
    } listGenessCommand;
  }
}
