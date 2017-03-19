//
//  add_gene_ontology_term.cpp
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
#include "../dba/gene_ontology.hpp"
#include "../datatypes/user.hpp"
#include "../extras/utils.hpp"
#include "../extras/serialize.hpp"

#include "../engine/commands.hpp"

namespace epidb {
  namespace command {

    class AddGeneOntologyTermCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::GENES, "Add a Gene Ontology Term to DeepBlue. A Gene Ontology Term refers to a term use to describe the genes functions.");
      }

      // GO:0070520,alpha4-beta1 integrin-CD81 complex,cellular_component,GO:0098797, 'userkey')
      static Parameters parameters_()
      {
        return {
          Parameter("go_id", serialize::STRING, "GO identifier"),
          Parameter("go_label", serialize::STRING, "GO label"),
          Parameter("description", serialize::STRING, "description of the Gene Ontology Term"),
          Parameter("namespace", serialize::STRING, "term namespace: cellular component, biological process or molecular function"),
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
      AddGeneOntologyTermCommand() : Command("add_gene_ontology_term", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        const std::string go_id = parameters[0]->as_string();
        const std::string go_label = parameters[1]->as_string();
        const std::string description = parameters[2]->as_string();
        const std::string go_namespace = parameters[3]->as_string();
        const std::string user_key = parameters[4]->as_string();

        std::string msg;
        datatypes::User user;

        if (!check_permissions(user_key, datatypes::INCLUDE_COLLECTION_TERMS, user, msg )) {
          result.add_error(msg);
          return false;
        }

        // Normal add_biosource code flow.
        std::string norm_description = utils::normalize_name(description);

        if (!dba::gene_ontology::is_valid_gene_ontology(go_id, go_label, go_namespace, msg)) {
          result.add_error(msg);
          return false;
        }

        std::string id;
        bool ret = dba::gene_ontology::add_gene_ontology_term(go_id, go_label,
                   description, norm_description,
                   go_namespace, user_key, id, msg);
        if (!ret) {
          result.add_error(msg);
        } else {
          result.add_string(id);
        }
        return ret;
      }
    } addGeneOntologyTermCommand;
  }
}
