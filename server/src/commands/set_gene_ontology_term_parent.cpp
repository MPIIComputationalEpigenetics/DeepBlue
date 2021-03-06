//
//  set_gene_ontology_term_parent.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 22.02.17.
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

#include "../dba/gene_ontology.hpp"
#include "../dba/users.hpp"
#include "../datatypes/user.hpp"
#include "../extras/utils.hpp"
#include "../extras/serialize.hpp"

#include "../engine/commands.hpp"

#include "../log.hpp"
#include "../errors.hpp"

namespace epidb {
  namespace command {

    class SetGeneOntologyTermParenttCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::GENES, "Define a BioSource as parent of another BioSource. This command is used to build the BioSources hierarchy. A BioSource refers to a term describing the origin of a given sample, such as a tissue or cell line.");
      }

      static  Parameters parameters_()
      {
        return {
            Parameter("parent_go_id", serialize::STRING, "parent GO identifier"),
            Parameter("parent_go_id", serialize::STRING, "parent GO identifier"),
          parameters::UserKey
        };
      }

      static Parameters results_()
      {
        return {};
      }

    public:
      SetGeneOntologyTermParenttCommand() : Command("set_gene_ontology_term_parent", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        const std::string bigger_scope = parameters[0]->as_string();
        const std::string smaller_scope = parameters[1]->as_string();
        const std::string user_key = parameters[2]->as_string();

        std::string msg;
        datatypes::User user;

        if (!check_permissions(user_key, datatypes::INCLUDE_COLLECTION_TERMS, user, msg )) {
          result.add_error(msg);
          return false;
        }

        if (!dba::gene_ontology::exists_gene_ontology_term(bigger_scope)) {
          msg = Error::m(ERR_INVALID_GENE_ONTOLOGY_TERM_ID, bigger_scope);
          result.add_error(msg);
          return false;
        }

        if (!dba::gene_ontology::exists_gene_ontology_term(smaller_scope)) {
          msg = Error::m(ERR_INVALID_GENE_ONTOLOGY_TERM_ID, smaller_scope);
          result.add_error(msg);
          return false;
        }

        if (dba::gene_ontology::set_go_parent(bigger_scope, smaller_scope, msg)) {
          result.add_string(bigger_scope);
          result.add_string(smaller_scope);
          return true;
        } else {
          result.add_error(msg);
          return false;
        }
      }
    } setGeneOntologyTermParenttCommand;
  }
}
