//
//  count_go_terms.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 02.03.2017.
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

#include <string>
#include <unordered_map>
#include <vector>

#include "../datatypes/gene_ontology_terms.hpp"
#include "../datatypes/regions.hpp"


namespace epidb {
  namespace algorithms {

    bool __count_go_terms(const Regions &regions,
                          std::unordered_map<std::string, size_t>& counts,
                          size_t& total_genes, size_t& total_found_go_terms,
                          std::string &msg)
    {
      for (const auto& region: regions) {
        if (region->has_gene_infos()) {
          total_genes++;
          const GeneRegion* gene_region = static_cast<const GeneRegion*>(region.get());

          const std::vector<datatypes::GeneOntologyTermPtr>& go_terms = gene_region->get_gene_ontology_terms();

          for (const auto& go_term: go_terms) {
            total_found_go_terms++;
            const std::string& go_id = go_term->go_id();
            counts[go_id]++;
          }
        }
      }
      return true;
    }

    bool count_go_terms(const ChromosomeRegionsList &chromosomeRegionsList,
                        std::unordered_map<std::string, size_t>& counts,
                        size_t& total_genes, size_t& total_found_go_terms,
                        std::string &msg)
    {
      total_genes = 0;
      for (auto &chromosomeRegions : chromosomeRegionsList) {
        if(!__count_go_terms(chromosomeRegions.second, counts, total_genes, total_found_go_terms, msg)) {
          return false;
        }
      }
      return true;
    }
  } // namespace algorithms
} // namespace epidb
