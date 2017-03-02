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

#include <iostream>
#include <limits>
#include <set>
#include <vector>

#include "accumulator.hpp"

#include "../datatypes/column_types_def.hpp"
#include "../datatypes/regions.hpp"

#include "../dba/experiments.hpp"
#include "../dba/key_mapper.hpp"
#include "../dba/metafield.hpp"

#include "../cache/column_dataset_cache.cpp"

#include "../extras/utils.hpp"

#include "../errors.hpp"
#include "../log.hpp"

namespace epidb {
  namespace algorithms {


    bool count_go_terms(Regions &regions, std::unordered_map<std::string, size_t>& counts, std::string &msg)
    {
      for (const auto& region: regions) {
        if (region->has_gene_infos()) {
          const GeneRegion* gene_region = static_cast<const GeneRegion*>(region.get());

          const std::vector<datatypes::GeneOntologyTermPtr>& go_terms = gene_region->get_gene_ontology_terms();

          for (const auto& go_term: go_terms) {
            const std::string& go_id = go_term->go_id();
            counts[go_id]++;
          }
        }
      }

      return true;

    }

    bool count_go_terms(ChromosomeRegionsList &data,
                        std::unordered_map<std::string, size_t>& counts, std::string &msg)
    {
      for (auto &datum : data) {
        if(!count_go_terms(datum.second, counts, msg)) {
          return false;
        }
      }
      return true;
    }
  } // namespace algorithms
} // namespace epidb
