//
//  gene_ontology_terms.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 01.03.2017.
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

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "gene_ontology_terms.hpp"

namespace epidb {
  namespace datatypes {

    std::unordered_map<std::string, GeneOntologyTermPtr> go_id_to_term;

    GeneOntologyTermPtr GeneOntolyTermsPool::get_go_term(const std::string& go_id, const std::string& go_label, const std::string& go_namespace)
    {
      GeneOntologyTermPtr term;

      auto found_term = go_id_to_term.find(go_id);
      if (go_id_to_term.find(go_id) == go_id_to_term.end()) {
        term = std::make_shared<GeneOntologyTerm>(go_id, go_label, go_namespace);
        go_id_to_term[go_id] = term;
      } else {
        term = found_term->second;
      }

      return term;
    }
  }
}
