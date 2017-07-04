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

#include "algorithms.hpp"

namespace epidb {
  namespace algorithms {

    ChromosomeRegions disjoin_regions(Regions &&regions_data, const std::string& chromosome)
    {
      Regions regions = Regions();

      RegionPtr& actual = regions_data[0];
      for (size_t i = 1; i < regions_data.size(); i++) {
        RegionPtr& next = regions_data[i];

        // If regions dont overlap:
        //  just store the actual region
        if (actual->end() <= next->start()) {
          regions.emplace_back(std::move(actual));
          actual = std::move(next);

          // If the actual region is inside the next region:
          //  we store the actual region and move the starting of the next region to the end of the actual
        } else if (actual->start() >= next->start() && actual->end() < next->end()) {
          next->set_start(actual->end());
          regions.emplace_back(std::move(actual));
          actual = std::move(next);

          // If the next region is inside the actual region:
          //  we store the next region and move the starting of the actual regiin to the end of the next
        } else if (actual->start() >= next->start() && actual->end() >= next->end()) {
          actual->set_start(next->end());
          regions.emplace_back(std::move(next));

          // If have the same boundaries:
          //  remove the actual and replace by next
        } else if (actual->start() == next->start() && actual->end() == next->end()) {
          actual.reset(nullptr);
          actual = std::move(next);

          // The regions overlap and actual is before next
        } else {
          // If overlap, create a new region
          regions.emplace_back(build_simple_region(actual->start(), next->start(), -1));

          if (actual->end() > next->end()) {
            actual->set_start(next->end());
            /// make another region here
            regions.emplace_back(std::move(next));
            next.reset(nullptr);

          } else if (actual->end() < next->end()) {// ac
            RegionPtr new_region = build_simple_region(next->start(), actual->end(), -1);
            regions.emplace_back(std::move(new_region));
            next->set_start(actual->end());
            actual.reset(nullptr);
            actual = std::move(next);

          } else if (actual->end() == next->end()) {
            actual.reset(nullptr);
            actual = std::move(next);

          } else {
            std::cerr <<"??????????????" << std::endl;
            std::cerr << actual->start() << " : " << actual->end() << "  -   " << next->start() << " : " << next->end() << std::endl;
          }
        }
      }

      return ChromosomeRegions(chromosome, std::move(regions));
    }


    bool disjoin(ChromosomeRegionsList &regions_data, ChromosomeRegionsList &disjoin_set)
    {
      // long times = clock();
      std::vector<std::future<ChromosomeRegions > > threads;
      std::vector<std::shared_ptr<ChromosomeRegionsList> > result_parts;

      for (const auto& chromosome_regions : regions_data) {
        auto t = std::async(std::launch::async, &disjoin_regions,
                            std::move(chromosome_regions.second), std::ref(chromosome_regions.first));

        threads.emplace_back(std::move(t));
      }

      for (size_t i = 0; i < threads.size(); ++i) {
        threads[i].wait();
        auto result = threads[i].get();
        disjoin_set.emplace_back(std::move(result));
      }

      // long diffticks = clock() - times;
      // "OVERLAP: " << ((diffticks) / (CLOCKS_PER_SEC / 1000)) << std::endl;
      return true;
    }

  }
}