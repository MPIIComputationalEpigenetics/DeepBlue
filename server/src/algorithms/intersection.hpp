//
//  intersection.hpp
//  epidb
//
//  Created by Fabian Reinartz on 29.08.13.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#ifndef EPIDB_ALGORITHMS_INTERSECTION_HPP
#define EPIDB_ALGORITHMS_INTERSECTION_HPP

#include "../regions.hpp"

namespace epidb {
  namespace algorithms {

    bool intersect(const ChromosomeRegionsList &regions_a, const ChromosomeRegionsList &regions_b,
                         ChromosomeRegionsList &intersections);

  } // namespace algorithms
} // namespace epidb

#endif



