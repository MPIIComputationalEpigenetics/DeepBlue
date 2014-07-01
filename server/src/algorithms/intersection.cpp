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

#include "utils/interval_tree.hpp"

#include "../regions.hpp"
#include "intersection.hpp"


namespace epidb {
  namespace algorithms {

    bool get_chromosome_regions(const ChromosomeRegionsList &qr, const std::string &chr,
                                      Regions &chr_regions)
    {
      ChromosomeRegionsList::const_iterator cit;
      for (cit = qr.begin(); cit != qr.end(); ++cit) {
        if (cit->first == chr) {
          chr_regions = cit->second;
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

    bool intersect(const ChromosomeRegionsList &regions_a, const ChromosomeRegionsList &regions_b,
                         ChromosomeRegionsList &intersections)
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
        std::vector<Interval<Region> > intervals;
        for (RegionsConstIterator rcit = chr_regions_b->begin(); rcit != chr_regions_b->end(); ++rcit) {
          intervals.push_back(Interval<Region>(rcit->start(), rcit->end(), *rcit)); // XXX: mem?
        }
        IntervalTree<Region> tree(intervals);

        Regions chr_intersections = build_regions();
        // find all overlaps of regions from set A in the tree
        for (RegionsConstIterator rcit = chr_regions_a->begin(); rcit != chr_regions_a->end(); ++rcit) {
          std::vector<Interval<Region> > overlaps;
          tree.findOverlapping(rcit->start(), rcit->end(), overlaps);

          // add overlaps to the total of intersections
          chr_intersections->reserve(intersections.size() + overlaps.size());

          std::vector<Interval<Region> >::const_iterator ocit;
          for (ocit = overlaps.begin(); ocit != overlaps.end(); ++ocit) {
            Region region = ocit->value;
            // change the region to only the actually intersecting part
            region.set_start(rcit->start() > region.start() ? rcit->start() : region.start());
            region.set_end(rcit->end() > region.end() ? region.end() : rcit->end());

            chr_intersections->push_back(region);
          }
        }
        // add found intersections to query result
        intersections.push_back(ChromosomeRegions(*cit, chr_intersections));
      }

      return true;
    }

  } // namespace algorithms
} // namespace epidb
