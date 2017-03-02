//
//  gene_ontology_terms.hpp
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

#ifndef GENE_ONTOLOGY_TERMS_HPP
#define GENE_ONTOLOGY_TERMS_HPP

#include <memory>
#include <string>
#include <vector>

namespace epidb {
  namespace datatypes {

    class GeneOntologyTerm {
        std::string _go_id;
        std::string _go_label;
        std::string _go_namespace;

        public:
        GeneOntologyTerm(const std::string& go_id, const std::string& go_label, const std::string& go_namespace) :
            _go_id(go_id),
            _go_label(go_label),
            _go_namespace(go_namespace) {}


            const std::string& go_id() {
              return _go_id;
            }

            const std::string& go_label() {
              return _go_label;
            }

            const std::string& go_namespace() {
              return _go_namespace;
            }
    };

    typedef std::shared_ptr<GeneOntologyTerm> GeneOntologyTermPtr;

    class GeneOntolyTermsPool {
      public:
        static GeneOntologyTermPtr get_go_term(const std::string& go_id, const std::string& go_label, const std::string& go_namespace);
    };
  }
}

#endif