//
//  merge.cpp
//  epidb
//
//  Created by Fabian Reinartz on 18.09.13.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include <algorithm>
#include <set>
#include <map>

#include <iostream>

#include "merge.hpp"

#include "../datatypes/regions.hpp"

namespace epidb {
  namespace algorithms {

    Regions merge_regions(Regions &regions_a, Regions &regions_b)
    {
      size_t size = regions_a.size() + regions_b.size();
      Regions results = build_regions(size);
      if (size == 0) {
        return results;
      }

      Regions sorted = build_regions(regions_a.size() + regions_b.size());
      sorted.insert(sorted.end(), std::make_move_iterator(regions_a.begin()), std::make_move_iterator(regions_a.end()));
      sorted.insert(sorted.end(), std::make_move_iterator(regions_b.begin()), std::make_move_iterator(regions_b.end()));
      std::sort(sorted.begin(), sorted.end(), RegionPtrComparer);

      // fill results and remove duplicates
      results.push_back(std::move(sorted[0]));

      for (auto it = sorted.begin() + 1; it != sorted.end(); ++it) {
        const AbstractRegion* last = results[results.size()-1].get();
        if ( (*it)->start() != last->start() || (*it)->end() != last->end() || (*it)->dataset_id() != last->dataset_id()) {
          results.push_back(std::move(*it));
        }
      }

      return results;
    }

    ChromosomeRegionsList merge_chromosome_regions(ChromosomeRegionsList &chrregions_a, ChromosomeRegionsList &chrregions_b)
    {
      std::cerr << "merge_chromosome_regions" << std::endl;

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
