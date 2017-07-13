//
//  gene_ontogy.hpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 21.02.2017
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

#ifndef DBA_GENE_ONTOLOGY_HPP
#define DBA_GENE_ONTOLOGY_HPP

#include <string>
#include <vector>

#include "../cache/connected_cache.hpp"

#include "../datatypes/user.hpp"

#include "../extras/utils.hpp"

namespace epidb {
  namespace dba {
    namespace gene_ontology {

      extern ConnectedCache go_cache;

      static const std::string GO_NAMESPACE_CELLULAR_COMPONENT;
      static const std::string GO_NAMESPACE_BIOLOGICAL_PROCESS;
      static const std::string GO_NAMESPACE_MOLECULAR_FUNCTION;

      bool is_valid_gene_ontology(const std::string &go_id,  const std::string &go_label,
                                  const std::string &go_namespace, std::string &msg);

      bool add_gene_ontology_term(const datatypes::User& user,
                                  const std::string &go_id,
                                  const std::string &go_label,
                                  const std::string &description, const std::string &norm_description,
                                  const std::string &go_namespace,
                                  std::string &gene_ontology_term_id, std::string &msg);

      bool exists_gene_ontology_term(const std::string &norm_go_id);

      bool annotate_gene(const std::string& gene_ensg_id, const std::string& go_id, std::string& msg);

      bool set_go_parent(const std::string& bigger_scope, const std::string& smaller_scope, std::string& msg);

      bool count_go_terms_in_genes(const std::vector<std::string> &chromosomes, const Position start, const Position end,
                                   const std::string& strand,
                                   const std::vector<std::string>& genes, const std::vector<std::string>& go_terms,
                                   const std::string& gene_model, const std::string& norm_gene_model,
                                   std::vector<utils::IdNameCount>& counts, size_t &total_go_terms,
                                   std::string& msg);
    }
  }
}

#endif /* defined(DBA_GENE_ONTOLOGY_HPP) */