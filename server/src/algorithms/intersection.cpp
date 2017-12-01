//
//  intersection.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Fabian Reinartz on 29.08.13.
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

#include <cmath>
#include <future>
#include <iostream>
#include <set>
#include <vector>
#include <thread>

#include "../datatypes/regions.hpp"

#include "../processing/processing.hpp"

#include "algorithms.hpp"

namespace epidb {
  namespace algorithms {

    ChromosomeRegions overlap_regions(Regions &&regions_data, Regions &&regions_overlap, const std::string& chromosome,
                                      const bool overlap, const double amount, const std::string amount_type,
                                      processing::StatusPtr status, std::string &msg);

    bool overlap(ChromosomeRegionsList &regions_data, ChromosomeRegionsList &regions_overlap,
                 const bool overlap, const double amount, const std::string amount_type,
                 processing::StatusPtr status,
                 ChromosomeRegionsList &overlaps, std::string &msg);

    bool get_chromosome_regions(ChromosomeRegionsList &qr, const std::string &chr, Regions &chr_regions)
    {
      for (auto cit = qr.begin(); cit != qr.end(); ++cit) {
        if (cit->first == chr) {
          chr_regions = std::move(cit->second);
          return true;
        }
      }
      return false;
    }

    ChromosomeRegions overlap_regions(Regions &&regions_data, Regions &&regions_overlap, const std::string& chromosome,
                                      const bool overlap, const double amount, const std::string amount_type,
                                      processing::StatusPtr status, std::string &msg)
    {
      processing::RunningOp runningOp = status->start_operation(processing::ALGORITHM_OVERLAP_CHROMOSOME,
                                        BSON(
                                          "total_regions_data" << (int) regions_data.size() <<
                                          "total_regions_overlap" << (int) regions_overlap.size() <<
                                          "chromosome" << chromosome
                                        ));
      Regions regions = Regions();

      if (processing::is_canceled(status, msg)) {
        return ChromosomeRegions(chromosome, std::move(regions));
      }


      bool dynamic_overlap_length = amount_type == "bp" ? false : true;


      size_t data_size = regions_data.size();
      size_t data_pos = 0;

      auto it_ranges = regions_overlap.begin();

      Length min_range_length = -1;
      Length min_data_length = -1;

      while (it_ranges != regions_overlap.end()) {

        if (dynamic_overlap_length) {
          min_range_length = ceil(static_cast<double>( (*it_ranges)->end() - (*it_ranges)->start() ) * (amount / 100));
        } else {
          min_range_length = static_cast<Length>(amount);
        }

        while ((data_pos < data_size) &&
               (regions_data[data_pos]->end() <= (*it_ranges)->start()) )  {

          if (dynamic_overlap_length) {
            min_data_length = ceil(static_cast<double>((regions_data[data_pos]->end() - regions_data[data_pos]->start())) * (amount / 100));
          } else {
            min_data_length = static_cast<Length>(amount);
          }

          if (!overlap) {
            Length distance = calculate_distance(regions_data[data_pos], *it_ranges);
            if ((distance >= min_range_length) && (distance >= min_data_length)) {
              regions.emplace_back(std::move(regions_data[data_pos]));
            }
          }
          data_pos++;
        }

        while ((data_pos < data_size) &&
               ((*it_ranges)->end() >= regions_data[data_pos]->start()) )  {

          if (((*it_ranges)->start() < regions_data[data_pos]->end()) &&
              ((*it_ranges)->end() > regions_data[data_pos]->start())) {

            if (overlap) {
              if (dynamic_overlap_length) {
                min_data_length = ceil(static_cast<double>((regions_data[data_pos]->end() - regions_data[data_pos]->start())) * (amount / 100));
              } else {
                min_data_length = static_cast<Length>(amount);
              }

              Length overlap_one = calculate_overlap(*it_ranges, regions_data[data_pos]);
              Length overlap_two = calculate_overlap(regions_data[data_pos], *it_ranges);

              if (((overlap_one >= min_range_length) || (overlap_two >= min_range_length)) &&
                  ((overlap_one >= min_data_length)  || (overlap_two >= min_data_length))) {

                regions.emplace_back(std::move(regions_data[data_pos]));
              }
            }
          } else {
            if (!overlap) {

              Length distance_one = calculate_distance(*it_ranges, regions_data[data_pos]);
              Length distance_two = calculate_distance(regions_data[data_pos], *it_ranges);

              if ((distance_one >= min_range_length) && (distance_two >= min_range_length) &&
                  (distance_one >= min_data_length)  && (distance_two >= min_data_length)) {

                regions.emplace_back(std::move(regions_data[data_pos]));
              }
            }
          }
          data_pos++;
        }

        it_ranges++;
      }

      // Distance to the last element of the regions_overlap
      if (!overlap) {
        while (data_pos < data_size) {
          if (dynamic_overlap_length) {
            min_data_length = ceil(static_cast<double>((regions_data[data_pos]->end() - regions_data[data_pos]->start())) * (amount / 100));
          } else {
            min_data_length = static_cast<Length>(amount);
          }

          Length distance = calculate_distance(regions_overlap.back(), regions_data[data_pos]);
          if (distance >= min_range_length) {
            regions.emplace_back(std::move(regions_data[data_pos]));
          }
          data_pos++;
        }
      }

      return ChromosomeRegions(chromosome, std::move(regions));
    }

    bool intersect(ChromosomeRegionsList &regions_data, ChromosomeRegionsList &regions_overlap,
                   processing::StatusPtr status, ChromosomeRegionsList &intersections, std::string &msg)
    {
      return overlap(regions_data, regions_overlap, true, 0.0, "bp", status, intersections, msg);
    }

    bool overlap(ChromosomeRegionsList &regions_data, ChromosomeRegionsList &regions_overlap,
                 const bool overlap, const double amount, const std::string amount_type,
                 processing::StatusPtr status,  ChromosomeRegionsList &overlaps, std::string &msg)
    {
      processing::RunningOp runningOp = status->start_operation(processing::ALGORITHM_OVERLAP);
      if (processing::is_canceled(status, msg)) {
        return false;
      }

      // long times = clock();
      std::set<std::string> chromosomes;
      merge_chromosomes(regions_data, regions_overlap, chromosomes);

      std::vector<std::future<ChromosomeRegions > > threads;
      std::vector<std::shared_ptr<ChromosomeRegionsList> > result_parts;

      for (const auto& chr : chromosomes) {
        Regions chr_regions_data;
        Regions chr_regions_overlap;

        bool has_data = get_chromosome_regions(regions_data, chr, chr_regions_data);
        bool has_overlap = get_chromosome_regions(regions_overlap, chr, chr_regions_overlap);

        // If is there no data, nothing to be made.
        if (!has_data) {
          continue;
        }

        // If I want overlaps, but nothing to overlap, nothing to be made.
        if (overlap && !has_overlap) {
          continue;
        }

        // If I do not want overlaps, but nothing to overlap to... Add them all!
        if (!overlap && !has_overlap) {
          overlaps.emplace_back(ChromosomeRegions(chr, std::move(chr_regions_data)));
          continue;
        }

        auto t = std::async(std::launch::async, &overlap_regions,
                            std::move(chr_regions_data), std::move(chr_regions_overlap), std::ref(chr),
                            std::ref(overlap), std::ref(amount), std::ref(amount_type), std::ref(status), std::ref(msg));

        threads.emplace_back(std::move(t));
      }

      for (size_t i = 0; i < threads.size(); ++i) {
        threads[i].wait();
        auto result = threads[i].get();
        if (!result.second.empty()) {
          overlaps.emplace_back(std::move(result));
        }
      }



      // long diffticks = clock() - times;
      // "OVERLAP: " << ((diffticks) / (CLOCKS_PER_SEC / 1000)) << std::endl;
      return true;
    }

  }
}