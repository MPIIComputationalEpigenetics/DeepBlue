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

namespace epidb {
  namespace dba {
    namespace gene_ontology {

      static const std::string GO_NAMESPACE_CELLULAR_COMPONENT;
      static const std::string GO_NAMESPACE_BIOLOGICAL_PROCESS;
      static const std::string GO_NAMESPACE_MOLECULAR_FUNCTION;

      bool is_valid_gene_ontology(const std::string &go_id, const std::string &norm_go_id,
                                  const std::string &go_label, const std::string &norm_go_label,
                                  const std::string &go_namespace, const std::string &norm_go_namespace,
                                  std::string &msg);

      bool add_gene_ontology_term(const std::string &go_id, const std::string &norm_go_id,
                                  const std::string &go_label, const std::string &norm_go_label,
                                  const std::string &description, const std::string &norm_description,
                                  const std::string &go_namespace, const std::string &norm_go_namespace,
                                  const std::string &user_key,
                                  std::string &gene_ontology_term_id, std::string &msg);
    }
  }
}

#endif /* defined(DBA_GENE_ONTOLOGY_HPP) */