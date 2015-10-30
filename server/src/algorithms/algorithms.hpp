//
//  algorithms.hpp
//  epidb
//
//  Created by Felipe Albrecht on 30.10.15.
//  Copyright (c) 2014,2015 Max Planck Institute for Computer Science. All rights reserved.
//

#ifndef EPIDB_ALGORITHMS_ALGORITHMS_HPP
#define EPIDB_ALGORITHMS_ALGORITHMS_HPP

#include <vector>
#include <set>
#include <iostream>

#include "../datatypes/regions.hpp"
#include "../processing/processing.hpp"

namespace epidb {
  namespace algorithms {
    bool aggregate(ChromosomeRegionsList &data, ChromosomeRegionsList &ranges, const std::string &field,
                   processing::StatusPtr status, ChromosomeRegionsList &regions, std::string &msg);

    bool flank(ChromosomeRegionsList &regions, const Offset start, const Length length, const bool use_strand,
               ChromosomeRegionsList &result, std::string &msg);

    bool intersect(ChromosomeRegionsList &regions_a, ChromosomeRegionsList &regions_b,
                   ChromosomeRegionsList &intersections);

    ChromosomeRegionsList merge_chromosome_regions(ChromosomeRegionsList& chrregions_a, ChromosomeRegionsList& chrregions_b);
  } // namespace algorithms
} // namespace epidb

#endif