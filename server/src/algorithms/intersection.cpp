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

#include <future>
#include <iostream>
#include <set>
#include <vector>
#include <thread>

#include "../datatypes/regions.hpp"

namespace epidb {
  namespace algorithms {

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

    bool merge_chromosomes(const ChromosomeRegionsList &regions_a, const ChromosomeRegionsList &regions_b,
                           std::set<std::string> &chromosomes)
    {
      ChromosomeRegionsList::const_iterator ait;
      for (ait = regions_a.begin(); ait != regions_a.end(); ++ait) {
        chromosomes.insert(ait->first);
      }
      ChromosomeRegionsList::const_iterator bit;
      for (bit = regions_b.begin(); bit != regions_b.end(); ++bit) {
        chromosomes.insert(bit->first);
      }

      return true;
    }

    ChromosomeRegions intersect_regions(Regions &&regions_data, Regions &&regions_overlap, const std::string& chromosome)
    {
      Regions regions = Regions();

      size_t data_size = regions_data.size();
      size_t data_pos = 0;

      auto it_ranges = regions_overlap.begin();

      while (it_ranges != regions_overlap.end()) {
        while ((data_pos < data_size) &&
               (regions_data[data_pos]->end() < (*it_ranges)->start()) )  {
          data_pos++;
        }

        while ((data_pos < data_size) &&
               ((*it_ranges)->end() >= regions_data[data_pos]->start()) )  {

          if (((*it_ranges)->start() < regions_data[data_pos]->end()) &&
              ((*it_ranges)->end() > regions_data[data_pos]->start())) {
            regions.emplace_back(std::move(regions_data[data_pos]));
          }
          data_pos++;
        }
        it_ranges++;
      }

      return ChromosomeRegions(chromosome, std::move(regions));
    }

    bool intersect(ChromosomeRegionsList &regions_data, ChromosomeRegionsList &regions_overlap, ChromosomeRegionsList &intersectionsi)
    {
      // long times = clock();

      std::set<std::string> chromosomes;
      merge_chromosomes(regions_data, regions_overlap, chromosomes);

      std::vector<std::future<ChromosomeRegions > > threads;
      std::vector<std::shared_ptr<ChromosomeRegionsList> > result_parts;

      for (const auto& chr : chromosomes) {
        Regions chr_regions_data;
        Regions chr_regions_overlap;
        if (!get_chromosome_regions(regions_data, chr, chr_regions_data) ||
            !get_chromosome_regions(regions_overlap, chr, chr_regions_overlap)) {
          continue;
        }

        auto t = std::async(std::launch::async, &intersect_regions,
                            std::move(chr_regions_data), std::move(chr_regions_overlap), std::ref(chr));

        threads.emplace_back(std::move(t));
      }

      for (size_t i = 0; i < threads.size(); ++i) {
        threads[i].wait();
        auto result = threads[i].get();
        if (!result.second.empty()) {
          intersectionsi.emplace_back(std::move(result));
        }
      }

      // long diffticks = clock() - times;
      // "INTERSECT: " << ((diffticks) / (CLOCKS_PER_SEC / 1000)) << std::endl;
      return true;
    }

    ChromosomeRegions overlap_regions(Regions &&regions_data, Regions &&regions_overlap, const std::string& chromosome,
                                      const bool overlap, const double amount, const std::string amount_type)
    {

      std::cerr << overlap << std::endl;
      
      Regions regions = Regions();

      bool type = amount_type == "bp" ? true : false;
      

      size_t data_size = regions_data.size();
      size_t data_pos = 0;

      auto it_ranges = regions_overlap.begin();

      while (it_ranges != regions_overlap.end()) {
        while ((data_pos < data_size) &&
               (regions_data[data_pos]->end() < (*it_ranges)->start()) )  {
                 if (!overlap) {                  
                   std::cerr << "NOT OVERLAP 1" << std::endl;
                   regions.emplace_back(std::move(regions_data[data_pos]));
                 }
          data_pos++;
        }

        while ((data_pos < data_size) &&
               ((*it_ranges)->end() >= regions_data[data_pos]->start()) )  {

          if (((*it_ranges)->start() < regions_data[data_pos]->end()) &&
              ((*it_ranges)->end() > regions_data[data_pos]->start())) {          
                // Only includes if it is 'overlaping' option
                if (overlap) {
                  std::cerr << "OVERLAP 2" << std::endl;
                  regions.emplace_back(std::move(regions_data[data_pos]));
                }
          } else {
            // Only includes if it is 'not overlapping' option
            if (!overlap) {
              std::cerr << "NOT OVERLAP 3" << std::endl;
              regions.emplace_back(std::move(regions_data[data_pos]));
            }
          }
          data_pos++;
        }
        it_ranges++;
      }

      if (!overlap) {
        while (data_pos < data_size) {
          std::cerr << "NOT OVERLAP 4" << std::endl;
          regions.emplace_back(std::move(regions_data[data_pos]));
          data_pos++;
        }          
      }

      return ChromosomeRegions(chromosome, std::move(regions));
    }

    bool overlap(ChromosomeRegionsList &regions_data, ChromosomeRegionsList &regions_overlap,
                 const bool overlap, const double amount, const std::string amount_type,
                 ChromosomeRegionsList &overlaps)
    {
      // long times = clock();
      std::set<std::string> chromosomes;
      merge_chromosomes(regions_data, regions_overlap, chromosomes);

      std::vector<std::future<ChromosomeRegions > > threads;
      std::vector<std::shared_ptr<ChromosomeRegionsList> > result_parts;

      for (const auto& chr : chromosomes) {
        Regions chr_regions_data;
        Regions chr_regions_overlap;
        if (!get_chromosome_regions(regions_data, chr, chr_regions_data) ||
            !get_chromosome_regions(regions_overlap, chr, chr_regions_overlap)) {
          continue;
        }

        auto t = std::async(std::launch::async, &overlap_regions,
                            std::move(chr_regions_data), std::move(chr_regions_overlap), std::ref(chr),
                            std::ref(overlap), std::ref(amount), std::ref(amount_type));

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