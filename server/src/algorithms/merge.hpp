//
//  intersection.hpp
//  epidb
//
//  Created by Fabian Reinartz on 11.09.13.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#ifndef EPIDB_ALGORITHMS_MERGE_HPP
#define EPIDB_ALGORITHMS_MERGE_HPP

#include "../datatypes/regions.hpp"

namespace epidb {
  namespace algorithms {

    bool merge_regions(Regions& regions_a, Regions& regions_b, Regions& results);

    ChromosomeRegionsList merge_chromosome_regions(ChromosomeRegionsList& chrregions_a, ChromosomeRegionsList& chrregions_b);

  } // namespace algorithms
} // namespace epidb

#endif