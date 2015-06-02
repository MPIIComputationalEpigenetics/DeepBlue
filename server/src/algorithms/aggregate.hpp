//
//  aggregate.hpp
//  epidb
//
//  Created by Felipe Albrecht on 14.04.14.
//  Copyright (c) 2014 Max Planck Institute for Computer Science. All rights reserved.
//

#ifndef EPIDB_ALGORITHMS_AGGREGATE_HPP
#define EPIDB_ALGORITHMS_AGGREGATE_HPP

#include <vector>
#include <set>
#include <iostream>

#include "utils/interval_tree.hpp"

#include "../datatypes/regions.hpp"
#include "../processing/processing.hpp"
#include "intersection.hpp"


namespace epidb {
  namespace algorithms {
    bool aggregate(ChromosomeRegionsList &data, ChromosomeRegionsList &ranges, const std::string &field,
                   processing::StatusPtr status, ChromosomeRegionsList &regions, std::string &msg);
  } // namespace algorithms
} // namespace epidb

#endif