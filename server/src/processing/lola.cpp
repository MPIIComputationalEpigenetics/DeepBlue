//
//  lola.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 23.06.2017.
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

#include "../dba/genomes.hpp"
#include "../dba/genes.hpp"
#include "../dba/gene_ontology.hpp"
#include "../dba/queries.hpp"
#include "../dba/retrieve.hpp"

#include "../extras/utils.hpp"


#include <algorithm> // for min and max

using namespace std;

namespace epidb {
  namespace processing {
    bool lola(const std::string& query_id, const std::string& universe_query_id,
              const mongo::BSONObj& databases,
              const std::string& genome,
              const std::string& user_key,
              processing::StatusPtr status, mongo::BSONObj& result, std::string& msg)
    {
      INIT_PROCESSING(PROCESS_LOLA, status)

      std::cerr << query_id << std::endl;

      std::cerr << universe_query_id << std::endl;

      ChromosomeRegionsList queryChromosomeRegionsList;
      if (!dba::query::retrieve_query(user_key, query_id, status, queryChromosomeRegionsList, msg)) {
        return false;
      }
      size_t total_query_regions = count_regions(queryChromosomeRegionsList);
      std::cerr << total_query_regions << std::endl;

      std::cerr << "LOADING UNIVERSE" << std::endl;
      long times = clock();
      ChromosomeRegionsList universeChromosomeRegionsList;
      if (!dba::query::retrieve_query(user_key, universe_query_id, status, universeChromosomeRegionsList, msg)) {
        return false;
      }
      size_t total_universe_regions = count_regions(universeChromosomeRegionsList);
      std::cerr << total_universe_regions << std::endl;
      long diffticks = clock() - times;
      std::cerr << "Load universe: " << ((diffticks) / (CLOCKS_PER_SEC / 1000)) << std::endl;

      std::set<std::string> chromosomes_s;
      const std::string& norm_genome = utils::normalize_name(genome);
      if (!dba::genomes::get_chromosomes(norm_genome, chromosomes_s, msg)) {
        return false;
      }
      std::vector<std::string> chromosomes(chromosomes_s.begin(), chromosomes_s.end());

      auto databases_it = databases.begin();
      while ( databases_it.more() ) {
        const mongo::BSONElement &database = databases_it.next();
        const std::string& database_name = std::string(database.fieldName());

        const auto& datasets = database.Obj();

        auto datasets_it = datasets.begin();
        while (datasets_it.more()) {
          long times = clock();
          const auto& experiment_name = datasets_it.next().str();
          std::cerr << experiment_name << " - ";

          mongo::BSONObj regions_query;
          if (!dba::query::build_experiment_query(-1, -1, experiment_name, regions_query, msg)) {
            return false;
          }

          ChromosomeRegionsList reg;
          if (!dba::retrieve::get_regions(genome, chromosomes, regions_query, false, status, reg, msg)) {
            return false;
          }

          size_t total = count_regions(reg);
          std::cerr << total << " ";

          size_t totalqueryChromosomeRegionsList = count_regions(queryChromosomeRegionsList);
          std::cerr << totalqueryChromosomeRegionsList << " ";

          size_t query_overlap_total;
          if (!algorithms::intersect_count(queryChromosomeRegionsList, reg, query_overlap_total)) {
            return false;
          }
          std::cerr << query_overlap_total << " ";

          size_t totaluniverseChromosomeRegionsList = count_regions(universeChromosomeRegionsList);
          std::cerr << totaluniverseChromosomeRegionsList << " ";

          size_t universe_overlap_total;
          if (!algorithms::intersect_count(universeChromosomeRegionsList, reg, universe_overlap_total)) {
            return false;
          }
          std::cerr << universe_overlap_total << " ";
          long diffticks = clock() - times;
          std::cerr << ((diffticks) / (CLOCKS_PER_SEC / 1000)) << std::endl;

        }
      }

      return true;
    }
  }
}