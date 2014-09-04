//
//  merge.cpp
//  epidb
//
//  Created by Fabian Reinartz on 18.09.13.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include "../regions.hpp"
#include "merge.hpp"

#include <algorithm>
#include <set>
#include <map>


namespace epidb {
  namespace algorithms {

    bool merge_regions(const Regions& regions_a, const Regions& regions_b, Regions& results)
    {
      results = build_regions();
      results->reserve(regions_a->size()); // least number of result elements

      // XXX: overall memory usage
      Regions sorted = build_regions();
      sorted->reserve(regions_a->size() + regions_b->size());
      sorted->insert(sorted->end(), regions_a->begin(), regions_a->end());
      sorted->insert(sorted->end(), regions_b->begin(), regions_b->end());

      std::sort(sorted->begin(), sorted->end());
      // fill results and remove duplicates
      Region last = *(sorted)->begin();
      results->push_back(last);

      for (RegionsIterator it = sorted->begin()+1; it != sorted->end(); ++it) {
        if (it->start() != last.start() || it->end() != last.end() || it->dataset_id() != last.dataset_id()) {
          results->push_back(*it);
        }
        last = *it;
      }
      return true;
    }

    bool merge_chromosome_regions(const ChromosomeRegionsList& chrregions_a, const ChromosomeRegionsList& chrregions_b,
               ChromosomeRegionsList& results)
    {
      // find out common chromosomes
      std::map<std::string, Regions> chromosomes_a;
      std::map<std::string, Regions> chromosomes_b;
      std::set<std::string> chromosomes;

      ChromosomeRegionsList::const_iterator qra;
      for (qra = chrregions_a.begin(); qra != chrregions_a.end(); ++qra) {
        chromosomes_a[qra->first] = qra->second;
      }
      ChromosomeRegionsList::const_iterator qrb;
      for (qrb = chrregions_b.begin(); qrb != chrregions_b.end(); ++qrb) {
        chromosomes_b[qrb->first] = qrb->second;
        // check if chromosome in common
        if (chromosomes_a.find(qrb->first) != chromosomes_a.end()) {
          chromosomes.insert(qrb->first);
        }
      }

      // merge common chromosomes
      std::set<std::string>::iterator cit;
      for (cit = chromosomes.begin(); cit != chromosomes.end(); ++cit) {
        const Regions& regions_a = chromosomes_a[*cit];
        const Regions& regions_b = chromosomes_b[*cit];

        Regions result;
        // merge regions for current chromosome
        if (!merge_regions(regions_a, regions_b, result)) {
          return false;
        }
        results.push_back(ChromosomeRegions(*cit, result));
      }

      // add chromosome data that are unique to one list
      std::map<std::string, Regions>::iterator cit_a;
      for (cit_a = chromosomes_a.begin(); cit_a != chromosomes_a.end(); ++cit_a) {
        // check if not a common chromosome
        if (chromosomes.find(cit_a->first) == chromosomes.end()) {
          results.push_back(ChromosomeRegions(cit_a->first, cit_a->second));
        }
      }
      std::map<std::string, Regions>::iterator cit_b;
      for (cit_b = chromosomes_b.begin(); cit_b != chromosomes_b.end(); ++cit_b) {
        // check if not a common chromosome
        if (chromosomes.find(cit_b->first) == chromosomes.end()) {
          results.push_back(ChromosomeRegions(cit_b->first, cit_b->second));
        }
      }

      return true;

      // for (qra = chrregions_a.begin(); qra != chrregions_a.end(); ++qra) {
      //   // find regions for matching chromosome in other result set
      //   bool match_found = false;
      //   ChromosomeRegionsList::const_iterator qrb;
      //   for (qrb = chrregions_b.begin(); qrb != chrregions_b.end(); ++qrb) {
      //     if (qrb->first.compare(qra->first) == 0) {
      //       match_found = true;
      //       break;
      //     }
      //   }
      //   if (!match_found) {
      //     continue;
      //   }
      //   const Regions& regions_a = qra->second;
      //   const Regions& regions_b = qrb->second;

      //   Regions result;
      //   // merge regions for current chromosome
      //   if (!merge_regions(regions_a, regions_b, result, msg)) {
      //     return false;
      //   }
      //   results.push_back(ChromosomeRegions(qra->first, result));
      // }
      // return true;
    }

  } // namespace algorithms
} // namespace epidb
