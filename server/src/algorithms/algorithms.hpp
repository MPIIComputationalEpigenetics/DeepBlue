//
//  algorithms.hpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 30.10.15.
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

    bool extend(ChromosomeRegionsList &regions, const Length length, const std::string direction, const bool use_strand,
                ChromosomeRegionsList &result, std::string &msg);

    bool flank(ChromosomeRegionsList &regions, const Offset start, const Length length, const bool use_strand,
               ChromosomeRegionsList &result, std::string &msg);

    bool intersect(ChromosomeRegionsList &regions_data, ChromosomeRegionsList &regions_overlap,
                   ChromosomeRegionsList &intersections);

   bool overlap(ChromosomeRegionsList &regions_data, ChromosomeRegionsList &regions_overlap,
                const bool overlap, const double amount, const std::string amount_type,
                ChromosomeRegionsList &intersections);

    ChromosomeRegionsList merge_chromosome_regions(ChromosomeRegionsList& chrregions_a, ChromosomeRegionsList& chrregions_b);
  } // namespace algorithms
} // namespace epidb

#endif