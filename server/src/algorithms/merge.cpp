//
//  merge.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Fabian Reinartz on 18.09.13.
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

#include <algorithm>
#include <set>
#include <map>

#include <iostream>

#include "../datatypes/regions.hpp"

namespace epidb {
  namespace algorithms {

    Regions merge_regions(Regions &regions_a, Regions &regions_b)
    {
      size_t size = regions_a.size() + regions_b.size();
      if (size == 0) {
        return Regions();
      }

      Regions results = Regions(size);
      Regions sorted = Regions(size);
      sorted.insert(sorted.end(), std::make_move_iterator(regions_a.begin()), std::make_move_iterator(regions_a.end()));
      sorted.insert(sorted.end(), std::make_move_iterator(regions_b.begin()), std::make_move_iterator(regions_b.end()));
      std::sort(sorted.begin(), sorted.end(), RegionPtrComparer);

      // fill results and remove duplicates
      results.emplace_back(std::move(sorted[0]));

      for (auto it = sorted.begin() + 1; it != sorted.end(); ++it) {
        const AbstractRegion* last = results[results.size()-1].get();
        if ( (*it)->start() != last->start() || (*it)->end() != last->end() || (*it)->dataset_id() != last->dataset_id()) {
          results.emplace_back(std::move(*it));
        }
      }

      return results;
    }

    ChromosomeRegionsList merge_chromosome_regions(ChromosomeRegionsList &chrregions_a, ChromosomeRegionsList &chrregions_b)
    {
      ChromosomeRegionsList results;
      // find out common chromosomes
      std::map<std::string, Regions> chromosomes_a;
      std::map<std::string, Regions> chromosomes_b;
      std::set<std::string> chromosomes;

      for (auto qra = chrregions_a.begin(); qra != chrregions_a.end(); ++qra) {
        chromosomes_a[qra->first] = std::move(qra->second);
      }

      for (auto qrb = chrregions_b.begin(); qrb != chrregions_b.end(); ++qrb) {
        chromosomes_b[qrb->first] = std::move(qrb->second);
        // check if chromosome in common
        if (chromosomes_a.find(qrb->first) != chromosomes_a.end()) {
          chromosomes.insert(qrb->first);
        }
      }

      // merge common chromosomes
      std::set<std::string>::iterator cit;
      for (cit = chromosomes.begin(); cit != chromosomes.end(); ++cit) {
        Regions &regions_a = chromosomes_a[*cit];
        Regions &regions_b = chromosomes_b[*cit];

        Regions result = merge_regions(regions_a, regions_b);
        results.push_back(ChromosomeRegions(*cit, std::move(result)));
      }

      // add chromosome data that are unique to one list
      for (auto cit_a = chromosomes_a.begin(); cit_a != chromosomes_a.end(); ++cit_a) {
        // check if not a common chromosome
        if (chromosomes.find(cit_a->first) == chromosomes.end()) {
          results.push_back(ChromosomeRegions(cit_a->first, std::move(cit_a->second)));
        }
      }

      for (auto cit_b = chromosomes_b.begin(); cit_b != chromosomes_b.end(); ++cit_b) {
        // check if not a common chromosome
        if (chromosomes.find(cit_b->first) == chromosomes.end()) {
          results.push_back(ChromosomeRegions(cit_b->first, std::move(cit_b->second)));
        }
      }

      return results;
    }

  } // namespace algorithms
} // namespace epidb
