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

#include <boost/math/distributions/hypergeometric.hpp>

#include "../algorithms/algorithms.hpp"

#include "../dba/genes.hpp"
#include "../dba/gene_ontology.hpp"
#include "../dba/queries.hpp"

#include "../extras/utils.hpp"


#include <algorithm> // for min and max

using namespace boost::math;
using namespace std;

double fisher_test(unsigned a, unsigned b, unsigned c, unsigned d)
{
  unsigned N = a + b + c + d;
  unsigned r = a + c;
  unsigned n = c + d;
  unsigned max_for_k = min(r, n);
  unsigned min_for_k = (unsigned)max(0, int(r + n - N));
  hypergeometric_distribution<> hgd(r, n, N);
  double cutoff = pdf(hgd, c);
  double tmp_p = 0.0;
  for(unsigned k = min_for_k; k < max_for_k + 1; k++) {
    double p = pdf(hgd, k);
    if(p <= cutoff) tmp_p += p;
  }
  return tmp_p;
}

namespace epidb {
  namespace processing {
    bool calculate_enrichment(const std::string& query_id, const std::string& gene_model,
                              const std::string& user_key,
                              processing::StatusPtr status, mongo::BSONObj& result, std::string& msg)
    {

      const std::string norm_gene_model = utils::normalize_name(gene_model);

      ChromosomeRegionsList chromosomeRegionsList;
      if (!dba::query::retrieve_query(user_key, query_id, status, chromosomeRegionsList, msg)) {
        return false;
      }

      std::vector<std::string> empty_chromosomes;
      std::vector<std::string> chromosomes_with_data;
      for (const auto& chromosomeRegions: chromosomeRegionsList) {
        chromosomes_with_data.push_back(chromosomeRegions.first);
      }
      std::vector<std::string> genes;

      size_t total_go_terms = 0;
      std::vector<utils::IdNameCount> counts;
      if (!dba::gene_ontology::count_go_terms_in_genes(empty_chromosomes,
          -1, -1, "", genes,
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
      if (!dba::genes::get_genes_from_database(chromosomes_with_data, -1, -1, "", genes, utils::normalize_name(gene_model), genesRegionsList, msg)) {
        return false;
      }

      size_t total_genes = 0;
      for (const auto& chromosome_region: genesRegionsList) {
        total_genes += chromosome_region.second.size();
      }

      ChromosomeRegionsList intersections;
      if (!algorithms::intersect(genesRegionsList, chromosomeRegionsList,intersections)) {
        return false;
      }

      size_t total_size = 0;
      for (const auto& chromosomeRegions: chromosomeRegionsList) {
        total_size += chromosomeRegions.second.size();
      }

      size_t total_found_genes = 0;
      size_t total_found_go_terms = 0;
      std::unordered_map<std::string, size_t> go_terms_counts;
      if (!algorithms::count_go_terms(intersections, go_terms_counts, total_found_genes, total_found_go_terms, msg)) {
        return false;
      }

      mongo::BSONArrayBuilder ab;
      for (const auto& kv: go_terms_counts) {
        unsigned aa = kv.second; // GO terms overlaped or overlap Genes with this GO annotation|
        unsigned bb = total_counts[kv.first] - aa; // Total genes with this Go terms in the gene model
        unsigned cc = total_found_genes - aa;
        unsigned dd = total_genes - (aa + bb + cc);

        mongo::BSONObjBuilder bob;
        bob.append("id", kv.first);
        bob.append("name", id_to_name[kv.first]);
        bob.append("go_colocated", (long long) total_counts[kv.first]);
        bob.append("ratio", (float(kv.second) / float(total_counts[kv.first])));
        bob.append("p_value", (fisher_test(aa, bb, cc, dd)));

        ab.append(bob.obj());
      }

      mongo::BSONObjBuilder bob;
      bob.append("total_genes", (long long) total_genes);
      bob.append("total_colocated_genes", (long long) total_found_genes);
      bob.append("total_annotated_go_terms", (long long) total_go_terms);
      bob.append("total_colocated_annotated_go_terms", (long long) total_go_terms);
      bob.append("go_terms", ab.arr());

      result = bob.obj();


      return true;
    }
  }
}