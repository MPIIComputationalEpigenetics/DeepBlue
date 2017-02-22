//
//  annotate_gene.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 21.02.17.
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

#include "../dba/controlled_vocabulary.hpp"
#include "../dba/dba.hpp"
#include "../dba/genes.hpp"
#include "../dba/gene_ontology.hpp"
#include "../datatypes/user.hpp"
#include "../extras/serialize.hpp"

#include "../engine/commands.hpp"

#include "../errors.hpp"

namespace epidb {
  namespace command {

    class AnnotateGene: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::GENES, "Annotate a Gene with a Gene Ontology Term.");
      }

      // GO:0070520,alpha4-beta1 integrin-CD81 complex,cellular_component,GO:0098797, 'userkey')
      static Parameters parameters_()
      {
        return {
          Parameter("gene_ensb_id", serialize::STRING, "Gene ENSB ID (ENSGXXXXXXXXXXX identifier"),
          Parameter("go_term_id", serialize::STRING, "GO Term ID"),
          parameters::UserKey
        };
      }

      static Parameters results_()
      {
        return {
          Parameter("id", serialize::STRING, "id of the newly inserted Gene Ontology Term")
        };
      }

    public:
      AnnotateGene() : Command("annotate_gene", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        const std::string gene_ensg_id = parameters[0]->as_string();
        const std::string go_term_id = parameters[1]->as_string();
        const std::string user_key = parameters[2]->as_string();

        std::string msg;
        datatypes::User user;

        if (!check_permissions(user_key, datatypes::INCLUDE_COLLECTION_TERMS, user, msg )) {
          result.add_error(msg);
          return false;
        }

        if (!dba::genes::exists_gene_ensg(gene_ensg_id)) {
          msg = Error::m(ERR_INVALID_GENE_ID, gene_ensg_id, "any");
          result.add_error(msg);
          return false;
        }

        if (!dba::gene_ontology::exists_gene_ontology_term(go_term_id)) {
          msg = Error::m(ERR_INVALID_GENE_ONTOLOGY_TERM_ID, go_term_id);
          result.add_error(msg);
          return false;
        }

        bool ret = dba::gene_ontology::annotate_gene(gene_ensg_id, go_term_id, msg);
        if (!ret) {
          result.add_error(msg);
        } else {
          result.add_string(go_term_id + " inserted in all " + gene_ensg_id);
        }
        return ret;
      }
    } annotateGene;
  }
}
