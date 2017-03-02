//
//  calculate_enrichment.cpp
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

#include <iterator>
#include <string>

#include "../algorithms/algorithms.hpp"

#include "../dba/gene_ontology.hpp"
#include "../dba/queries.hpp"

#include "../extras/utils.hpp"


namespace epidb {
  namespace processing {
    bool calculate_enrichment(const std::string& query_id, const std::string& gene_model,
                              const std::string& user_key,
                              processing::StatusPtr status, mongo::BSONObj& result, std::string& msg)
    {

      const std::string norm_gene_model = utils::normalize_name(gene_model);

      std::vector<utils::IdNameCount> counts;
      if (!dba::gene_ontology::count_go_terms_in_genes(gene_model, norm_gene_model, counts, msg)) {
        return false;
      }

      std::unordered_map<std::string, size_t> total_counts;
      for (const auto& id_name_count: counts) {
        total_counts[id_name_count.id] = id_name_count.count;
      }

      ChromosomeRegionsList chromosomeRegionsList;
      if (!dba::query::retrieve_query(user_key, query_id, status, chromosomeRegionsList, msg)) {
        return false;
      }

      size_t total_size = 0;
      for (const auto& chromosomeRegions: chromosomeRegionsList) {
        total_size += chromosomeRegions.second.size();
      }

      std::unordered_map<std::string, size_t> regions_counts;
      if (!algorithms::count_go_terms(chromosomeRegionsList, regions_counts, msg)) {
        return false;
      }

      return true;
    }
  }
}