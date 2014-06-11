//
//  intersection.hpp
//  epidb
//
//  Created by Fabian Reinartz on 11.09.13.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#ifndef EPIDB_ALGORITHMS_MERGE_HPP
#define EPIDB_ALGORITHMS_MERGE_HPP

#include "../regions.hpp"

namespace epidb {
  namespace algorithms {

    bool merge_regions(const Regions& regions_a, const Regions& regions_b, Regions& results);

    bool merge_chromosome_regions(const ChromosomeRegionsList& chrregions_a, const ChromosomeRegionsList& chrregions_b,
               ChromosomeRegionsList& results);

  } // namespace algorithms
} // namespace epidb

#endif