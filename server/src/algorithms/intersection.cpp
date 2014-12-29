//
//  intersection.cpp
//  epidb
//
//  Created by Fabian Reinartz on 29.08.13.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include <vector>
#include <set>
#include <iostream>

#include "intersection.hpp"

#include "utils/interval_tree.hpp"

#include "../datatypes/regions.hpp"

namespace epidb {
  namespace algorithms {

    bool get_chromosome_regions(ChromosomeRegionsList &qr, const std::string &chr, Regions &chr_regions)
    {
      for (auto cit = qr.begin(); cit != qr.end(); ++cit) {
        if (cit->first == chr) {
          chr_regions = std::move(cit->second);
          return true;
        }
      }
      return false;
    }

    bool merge_chromosomes(const ChromosomeRegionsList &regions_a, const ChromosomeRegionsList &regions_b,
                           std::set<std::string> &chromosomes)
    {
      ChromosomeRegionsList::const_iterator ait;
      for (ait = regions_a.begin(); ait != regions_a.end(); ++ait) {
        chromosomes.insert(ait->first);
      }
      ChromosomeRegionsList::const_iterator bit;
      for (bit = regions_b.begin(); bit != regions_b.end(); ++bit) {
        chromosomes.insert(bit->first);
      }

      return true;
    }

    bool intersect(ChromosomeRegionsList &regions_a, ChromosomeRegionsList &regions_b, ChromosomeRegionsList &intersections)
    {
      // get intersection of regions on equal ChromosomeRegions
      std::set<std::string> chromosomes;
      merge_chromosomes(regions_a, regions_b, chromosomes);

      std::set<std::string>::const_iterator cit;
      for (cit = chromosomes.begin(); cit != chromosomes.end(); ++cit) {

        Regions chr_regions_a, chr_regions_b;
        if (!get_chromosome_regions(regions_a, *cit, chr_regions_a) ||
            !get_chromosome_regions(regions_b, *cit, chr_regions_b)) {
          // XXX: is this an error?
          continue;
        }

        // build an interval tree of region set B
        std::vector<Interval<RegionPtr> > intervals;
        for (auto &region_b : chr_regions_b) {
          Interval<RegionPtr> interval(region_b->start(), region_b->end(), std::move(region_b));
          intervals.push_back(std::move(interval));
        }
        IntervalTree<RegionPtr> tree(intervals);

        Regions chr_intersections = build_regions();
        // find all overlaps of regions from set A in the tree
        for (auto &region_a : chr_regions_a) {
          std::vector<RegionPtr> overlaps;
          tree.findOverlapping(region_a->start(), region_a->end(), overlaps);

          // add overlaps to the total of intersections
          chr_intersections.reserve(intersections.size() + overlaps.size());

          std::vector<Interval<RegionPtr> >::const_iterator ocit;
          for (auto &regions_b :  overlaps) {
            regions_b->set_start(region_a->start() > regions_b->start() ? region_a->start() : regions_b->start());
            regions_b->set_end(region_a->end() > regions_b->end() ? regions_b->end() : region_a->end());
            chr_intersections.push_back(std::move(regions_b));
          }
        }
        // add found intersections to query result
        intersections.push_back(ChromosomeRegions(*cit, std::move(chr_intersections)));
      }

      return true;
    }

  } // namespace algorithms
} // namespace epidb
