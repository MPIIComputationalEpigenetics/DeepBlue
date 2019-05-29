//
//  merge.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Fabian Reinartz on 18.09.13.
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

#include <algorithm>
#include <set>
#include <map>

#include <iostream>

#include "../cache/column_dataset_cache.hpp"

#include "../datatypes/regions.hpp"

#include "../dba/column_types.hpp"
#include "../dba/experiments.hpp"

namespace epidb {
  namespace algorithms {

    bool chr_regions_flank(const ChromosomeRegions &chr_regions, const Offset start, const Length length, const bool use_strand,
                           ChromosomeRegionsList &result, std::string &msg)
    {
      const std::string& chromosome = chr_regions.first;
      const Regions& regions = chr_regions.second;
      Regions result_regions = Regions(regions.size());


      for (const RegionPtr& region : regions) {
        dba::columns::ColumnTypePtr column_type;

        bool positve_strand = true;
        if (use_strand) {
          int pos;
          // If the dataset does not have STRAND column, use positive strand
          if (!cache::get_column_position_from_dataset(region->dataset_id(), "STRAND", pos, msg)) {
            positve_strand = true;
          }
          std::string strand = region->get_string(pos);
          if (strand == "-") {
            positve_strand = false;
          }
        }

        Position a;
        Position b;

        RegionPtr flank = region->clone();
        if (positve_strand) {
          if (start >= 0) {
            a = std::max<signed int>(0, region->end() + start);
            b = std::max<signed int>(0, region->end() + start + length);
          } else {
            a = std::max<signed int>(0, region->start() + start + length);
            b = std::max<signed int>(0, region->start() + start);
          }
        } else {
          if (start >= 0) {
            // We use signed int because otherwise the underflow will be a positive integer.
            a = std::max<signed int>(0, region->start() - start - length);
            b = std::max<signed int>(0, region->start() - start);
          } else {
            // We use signed int because otherwise the underflow will be a positive integer.
            a = std::max<signed int>(0, region->end() - start);
            b = std::max<signed int>(0, region->end() - start - length);
          }
        }

        if (a < b) {
          flank->set_start(a);
          flank->set_end(b);
        } else {
          flank->set_start(b);
          flank->set_end(a);
        }

        result_regions.emplace_back(std::move(flank));
      }

      result.emplace_back(chromosome, std::move(result_regions));

      return true;
    }

    bool flank(ChromosomeRegionsList &regions, const Offset start, const Length length, const bool use_strand,
               ChromosomeRegionsList &result, std::string &msg)
    {
      for (const auto &chr_region : regions) {
        if (!chr_regions_flank(chr_region, start, length, use_strand, result, msg)) {
          return false;
        }
      }
      return true;
    }
  } // namespace algorithms
} // namespace epidb
