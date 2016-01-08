//
//  intersection.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Fabian Reinartz on 29.08.13.
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

#include <vector>
#include <set>
#include <iostream>

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

      for (const auto& chr: chromosomes) {

        Regions chr_regions_a, chr_regions_b;
        if (!get_chromosome_regions(regions_a, chr, chr_regions_a) ||
            !get_chromosome_regions(regions_b, chr, chr_regions_b)) {
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

        Regions chr_intersections = Regions();
        // find all overlaps of regions from set A in the tree
        for (auto &region_a : chr_regions_a) {
          std::vector<RegionPtr> overlaps;
          tree.findOverlapping(region_a->start(), region_a->end(), overlaps);

          if (!overlaps.empty()) {
            chr_intersections.emplace_back(std::move(region_a));
          }
        }
        // add found intersections to query result
        intersections.push_back(ChromosomeRegions(chr, std::move(chr_intersections)));
      }

      return true;
    }

  } // namespace algorithms
} // namespace epidb
