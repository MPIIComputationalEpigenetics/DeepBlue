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

#include "../dba/genes.hpp"
#include "../dba/gene_ontology.hpp"
#include "../dba/queries.hpp"

#include "../extras/utils.hpp"


#include <algorithm> // for min and max

using namespace std;

namespace epidb {
  namespace processing {
    bool lola(const std::string& query_id, const std::string& universe_query_id, const mongo::BSONObj& datasets,
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

      ChromosomeRegionsList universeChromosomeRegionsList;
      if (!dba::query::retrieve_query(user_key, universe_query_id, status, universeChromosomeRegionsList, msg)) {
        return false;
      }

      size_t total_universe_regions = count_regions(universeChromosomeRegionsList);
      std::cerr << total_universe_regions << std::endl;

      return true;
    }
  }
}