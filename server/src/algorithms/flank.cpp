//
//  merge.cpp
//  epidb
//
//  Created by Fabian Reinartz on 18.09.13.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
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

    bool chr_regions_flank(const ChromosomeRegions &chr_regions, const Offset start, const Length length, const bool use_strand,
                           ChromosomeRegionsList &result, std::string &msg)
    {
      const std::string& chromosome = chr_regions.first;
      const Regions& regions = chr_regions.second;
      Regions result_regions = build_regions(regions.size());


      for (const RegionPtr& region : regions) {
        dba::columns::ColumnTypePtr column_type;

        bool positve_strand = true;
        if (use_strand) {
          if (!dba::experiments::get_field_pos(region->dataset_id(), "STRAND", column_type, msg)) {
            return false;
          }
          std::string strand = region->get_string(column_type->pos());
          std::cerr << "strand: " << strand << std::endl;
          if (strand == "-") {
            positve_strand = false;
          }
        }

        Position a;
        Position b;

        RegionPtr flank = region->clone();
        if (positve_strand) {
          std::cerr << " positive " ;
          if (start >= 0) {
            std::cerr << " >= 0";
            a = region->end() + start;
            b = region->end() + start + length;
          } else {
            std::cerr << " < 0";
            a = region->start() + start + length;
            b = region->start() + start;
          }
        } else {
          std::cerr << " negative " ;
          if (start >= 0) {
            std::cerr << " >= 0";
            a = region->start() - start - length;
            b = region->start()  - start;
          } else {
            std::cerr << " < 0";
            a = region->end() - start;
            b = region->end() - start + length;
          }
        }
        std::cerr << std::endl;

        if (a < b) {
          flank->set_start(a);
          flank->set_end(b);
        } else {
          flank->set_start(b);
          flank->set_end(a);
        }

        result_regions.push_back(std::move(flank));
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
