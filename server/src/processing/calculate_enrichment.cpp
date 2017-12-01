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

#include "../dba/genes.hpp"
#include "../dba/gene_ontology.hpp"
#include "../dba/queries.hpp"

#include "../extras/utils.hpp"

#include "../extras/math.hpp"

#include "processing.hpp"

namespace epidb {
  namespace processing {
    bool calculate_enrichment(const datatypes::User& user,
                              const std::string& query_id, const std::string& gene_model,
                              processing::StatusPtr status, mongo::BSONObj& result, std::string& msg)
    {
      IS_PROCESSING_CANCELLED(status);
      processing::RunningOp runningOp =  status->start_operation(PROCESS_CALCULATE_GO_ENRICHMENT);

      const std::string norm_gene_model = utils::normalize_name(gene_model);

      ChromosomeRegionsList chromosomeRegionsList;
      if (!dba::query::retrieve_query(user, query_id, status, chromosomeRegionsList, msg)) {
        return false;
      }

      std::vector<std::string> empty_chromosomes;
      std::vector<std::string> chromosomes_with_data;
      for (const auto& chromosomeRegions: chromosomeRegionsList) {
        chromosomes_with_data.push_back(chromosomeRegions.first);
      }
      std::vector<std::string> genes;
      std::vector<std::string> go_terms;

      size_t total_go_terms = 0;
      std::vector<utils::IdNameCount> counts;
      if (!dba::gene_ontology::count_go_terms_in_genes(empty_chromosomes,
          -1, -1, "", genes, go_terms,
          gene_model, utils::normalize_name(gene_model),
          counts, total_go_terms,  msg)) {
        return false;
      }

      std::unordered_map<std::string, size_t> total_counts;
      std::unordered_map<std::string, std::string> id_to_name;
      for (const auto& id_name_count: counts) {
        total_counts[id_name_count.id] = id_name_count.count;
        id_to_name[id_name_count.id] = id_name_count.name;
      }

      ChromosomeRegionsList genesRegionsList;
      if (!dba::genes::get_genes_from_database(chromosomes_with_data, -1, -1, "", genes, go_terms, utils::normalize_name(gene_model), genesRegionsList, msg)) {
        return false;
      }

      size_t total_genes = 0;
      for (const auto& chromosome_region: genesRegionsList) {
        total_genes += chromosome_region.second.size();
      }

      ChromosomeRegionsList intersections;
      if (!algorithms::intersect(genesRegionsList, chromosomeRegionsList, status, intersections, msg)) {
        return false;
      }

      size_t total_size = count_regions(chromosomeRegionsList);

      size_t total_found_genes = 0;
      size_t total_overlaped_go_terms = 0;
      std::unordered_map<std::string, size_t> go_terms_counts;
      if (!algorithms::count_go_terms(intersections, go_terms_counts, total_found_genes, total_overlaped_go_terms, msg)) {
        return false;
      }

      size_t total_distinct_go_terms = go_terms_counts.size();

      mongo::BSONArrayBuilder ab;
      for (const auto& kv: go_terms_counts) {
        unsigned aa = kv.second; // GO terms overlaped or overlap Genes with this GO annotation|
        unsigned bb = total_counts[kv.first] - aa; // Total genes with this Go terms in the gene model
        unsigned cc = total_found_genes - aa;
        unsigned dd = total_genes - (aa + bb + cc);

        mongo::BSONObjBuilder bob;
        bob.append("id", kv.first);
        bob.append("name", id_to_name[kv.first]);
        bob.append("go_overlap", (long long) kv.second);
        bob.append("go_total", (long long) total_counts[kv.first]);
        bob.append("ratio", (float(kv.second) / float(total_counts[kv.first])));
        bob.append("p_value", (math::fisher_test(aa, bb, cc, dd)));

        ab.append(bob.obj());
      }

      mongo::BSONObjBuilder bob;
      bob.append("total_genes", (long long) total_genes);
      bob.append("total_colocated_genes", (long long) total_found_genes);
      bob.append("total_annotated_go_terms", (long long) total_go_terms);
      bob.append("total_colocated_annotated_go_terms", (long long) total_overlaped_go_terms);
      bob.append("total_distinct_colocated_annotated_go_terms", (long long) total_distinct_go_terms);
      bob.append("go_terms", ab.arr());

      result = bob.obj();


      return true;
    }
  }
}