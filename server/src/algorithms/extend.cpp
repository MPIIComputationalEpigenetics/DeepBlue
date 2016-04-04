//
//  extend.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Fabian Reinartz on 04.04.16.
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

#include "../datatypes/regions.hpp"

#include "../dba/column_types.hpp"
#include "../dba/experiments.hpp"

namespace epidb {
  namespace algorithms {

    enum Direction {
      FORWARD = 0,
      BACKWARD = 1,
      BOTH = 2
    };

    bool chr_regions_extend(const ChromosomeRegions &chr_regions, const Length length, const Direction direction, const bool use_strand,
                            ChromosomeRegionsList &result, std::string &msg)
    {
      const std::string& chromosome = chr_regions.first;
      const Regions& regions = chr_regions.second;
      Regions result_regions = Regions(regions.size());


      for (const RegionPtr& region : regions) {
        dba::columns::ColumnTypePtr column_type;

        bool positve_strand = true;
        if (use_strand) {
          if (!dba::experiments::get_field_pos(region->dataset_id(), "STRAND", column_type, msg)) {
            return false;
          }
          std::string strand = region->get_string(column_type->pos());
          if (strand == "-") {
            positve_strand = false;
          }
        }

        Position a;
        Position b;

        RegionPtr extend = region->clone();

        if (positve_strand) {
          if (direction == Direction::FORWARD) {
            a = region->start();
            b = region->end() + length;
          } else if (direction == Direction::BACKWARD) {
            // We use signed int because otherwise the underflow will be a positive integer.
            a = std::max<signed int>(0, region->start() - length);
            b = region->end();
          } else { // direction == BOTH
            // We use signed int because otherwise the underflow will be a positive integer.
            a = std::max<signed int>(0, region->start() - length);
            b = region->end() + length;
          }
        } else { // Negative strand
          if (direction == Direction::FORWARD) {
            // We use signed int because otherwise the underflow will be a positive integer.
            a = std::max<signed int>(0, region->start() - length);
            b = region->end();
          } else if (direction == Direction::BACKWARD) {
            a = region->start();
            b = region->end() + length;
          } else { // direction == BOTH
            // We use signed int because otherwise the underflow will be a positive integer.
            a = std::max<Position>(0, region->start() - length);
            b = region->end();
          }
        }

        if (a < b) {
          extend->set_start(a);
          extend->set_end(b);
        } else {
          extend->set_start(b);
          extend->set_end(a);
        }

        result_regions.emplace_back(std::move(extend));
      }

      result.emplace_back(chromosome, std::move(result_regions));

      return true;
    }

    bool extend(ChromosomeRegionsList &regions, const Length length, const std::string direction, const bool use_strand,
                ChromosomeRegionsList &result, std::string &msg)
    {
      Direction d;
      if (direction == "forward") {
        d = Direction::FORWARD;
      } else if (direction == "backward") {
        d = Direction::BACKWARD;
      } else {
        d = Direction::BOTH;
      }

      for (const auto &chr_region : regions) {
        if (!chr_regions_extend(chr_region, length, d, use_strand, result, msg)) {
          return false;
        }
      }
      return true;
    }
  } // namespace algorithms
} // namespace epidb
