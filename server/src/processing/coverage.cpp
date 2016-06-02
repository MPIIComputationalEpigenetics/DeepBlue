//
//  coverage.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 01.06.16.
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
#include <vector>

#include "../datatypes/regions.hpp"

#include "../dba/queries.hpp"
#include "../dba/genomes.hpp"

namespace epidb {
  namespace processing {

    bool coverage(const std::string &query_id, const std::string &genome, const std::string &user_key, processing::StatusPtr status, std::vector<CoverageInfo> &coverage_infos, std::string &msg)
    {
      ChromosomeRegionsList chromosomeRegionsList;
      if (!dba::query::retrieve_query(user_key, query_id, status, chromosomeRegionsList, msg)) {
        return false;
      }

      dba::genomes::GenomeInfoPtr gi;
      if (!dba::genomes::get_genome_info(genome,  gi, msg)) {
        return false;
      }

      for (ChromosomeRegionsList::iterator it = chromosomeRegionsList.begin();
           it != chromosomeRegionsList.end(); it++) {
        std::string &chromosome = it->first;
        Regions &regions = it->second;

        if (regions.empty()) {
          continue;
        }

        size_t chromosome_size;
        if (!dba::genomes::chromosome_size(genome, chromosome, chromosome_size, msg)) {
          return false;
        }

        size_t total = 0;
        Position last_pos = 0;
        for (auto const &region : regions) {
          Position init = std::max(last_pos, region->start());
          if (init < region->end()) {
            total += (region->end() - init);
            last_pos = region->end();
          }
        }

        std::cerr << chromosome << " " << total << " " << last_pos << " " << std::endl;
        coverage_infos.emplace_back(CoverageInfo{chromosome, chromosome_size, total});
      }

      return true;
    }
  }
}