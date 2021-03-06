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
#include <unordered_map>

#include "../datatypes/regions.hpp"
#include "../processing/processing.hpp"

namespace epidb {
  namespace algorithms {

    bool merge_chromosomes(const ChromosomeRegionsList &regions_a, const ChromosomeRegionsList &regions_b,
                           std::set<std::string> &chromosomes);

    Length calculate_distance(const RegionPtr& r1, const RegionPtr& r2);

    Length calculate_overlap(const RegionPtr& r1, const RegionPtr& r2);

    bool aggregate(ChromosomeRegionsList &data, ChromosomeRegionsList &ranges, const std::string &field,
                   processing::StatusPtr status, ChromosomeRegionsList &regions, std::string &msg);

    bool extend(ChromosomeRegionsList &regions, const Length length, const std::string direction, const bool use_strand,
                ChromosomeRegionsList &result, std::string &msg);

    bool count_go_terms(const ChromosomeRegionsList &chromosomeRegionsList,
                        std::unordered_map<std::string, size_t>& counts,
                        size_t& total_genes, size_t& total_found_go_terms,
                        std::string &msg);

    bool flank(ChromosomeRegionsList &regions, const Offset start, const Length length, const bool use_strand,
               ChromosomeRegionsList &result, std::string &msg);

    bool intersect(ChromosomeRegionsList &regions_data, ChromosomeRegionsList &regions_overlap,
                   processing::StatusPtr status, ChromosomeRegionsList &intersections, std::string &msg);

    bool overlap(ChromosomeRegionsList &regions_data, ChromosomeRegionsList &regions_overlap,
                 const bool overlap, const double amount, const std::string amount_type,
                 processing::StatusPtr status,  ChromosomeRegionsList &overlaps, std::string &msg);

    bool intersect_count(const ChromosomeRegionsList &regions_data, const ChromosomeRegionsList &regions_overlap,
                         size_t& count);

    bool overlap_count(const ChromosomeRegionsList &regions_data, const ChromosomeRegionsList &regions_overlap,
                       const bool overlap, const double amount, const std::string amount_type,
                       size_t& count);

    ChromosomeRegionsList disjoin(ChromosomeRegionsList &&regions_data);

    ChromosomeRegionsList merge_chromosome_regions(ChromosomeRegionsList& chrregions_a, ChromosomeRegionsList& chrregions_b);
  } // namespace algorithms
} // namespace epidb

#endif