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

      if (regions_data.empty()) {
        return ChromosomeRegions(chromosome, std::move(regions));
      }

      RegionPtr&& actual = std::move(regions_data[0]);
      for (size_t i = 1; i < regions_data.size(); i++) {
        RegionPtr&& next = std::move(regions_data[i]);

        // The regions have the same boundaries:
        if (actual->start() == next->start() && actual->end() == next->end()) {
          actual.reset(nullptr);
          actual = std::move(next);

          // Regions dont overlap:
        } else if (actual->end() <= next->start()) {
          regions.emplace_back(std::move(actual));
          actual = std::move(next);

          // The actual region is inside the next region:
        } else if (actual->start() >= next->start() && actual->end() < next->end()) {
          if (actual->start() > next->start()) {
            RegionPtr new_region = build_simple_region(next->start(), actual->start(), -1);
            regions.emplace_back(std::move(new_region));
          }
          next->set_start(actual->end());
          regions.emplace_back(std::move(actual));
          actual = std::move(next);

          // The actual is after the next region and is overlaped:
        } else if (actual->start() >= next->start() && actual->end() > next->end()) {
          if (actual->start() > next->start()) {
            RegionPtr pre_new_region = build_simple_region(next->start(), actual->start(), -1);
            regions.emplace_back(std::move(pre_new_region));
          }
          RegionPtr mid_new_region = build_simple_region(actual->start(), next->end(), -1);
          regions.emplace_back(std::move(mid_new_region));
          actual->set_start(next->end());

          // The actual is after but end in the same place
        } else if (actual->start() > next->start() && actual->end() == next->end()) {
          RegionPtr pre_new_region = build_simple_region(next->start(), actual->start(), -1);
          regions.emplace_back(std::move(pre_new_region));

          // The next region is inside the actual region:
        } else if (actual->start() <= next->start() && actual->end() > next->end()) {
          if (actual->start() < next->start()) {
            RegionPtr pre_new_region = build_simple_region(actual->start(), next->start(), -1);
            regions.emplace_back(std::move(pre_new_region));
          }
          actual->set_start(next->end());
          regions.emplace_back(std::move(next));

          // If the actual is before and overlap with the next
        } else if (actual->start() <= next->start() && actual->end() < next->end()) {
          if (actual->start() < next->start()) {
            RegionPtr pre_new_region = build_simple_region(actual->start(), next->start(), -1);
            regions.emplace_back(std::move(pre_new_region));
          }
          RegionPtr mid_new_region = build_simple_region(next->start(), actual->end(), -1);
          regions.emplace_back(std::move(mid_new_region));
          next->set_start(actual->end());
          actual = std::move(next);

          // if the actual is before and end in the same place
        } else if (actual->start() <= next->start() && actual->end() == next->end()) {
          RegionPtr pre_new_region = build_simple_region(actual->start(), next->start(), -1);
          regions.emplace_back(std::move(pre_new_region));
          actual->set_start(next->start());

        } else {
          std::cerr <<"??????????????" << std::endl;
          std::cerr << actual->start() << " : " << actual->end() << "  -   " << next->start() << " : " << next->end() << std::endl;
        }
      }
      regions.emplace_back(std::move(actual));

      std::sort(regions.begin(), regions.end(), RegionPtrComparer);

      return ChromosomeRegions(chromosome, std::move(regions));
    }

    ChromosomeRegionsList disjoin(ChromosomeRegionsList &&regions_data)
    {
      // long times = clock();
      std::vector<std::future<ChromosomeRegions > > threads;
      std::vector<std::shared_ptr<ChromosomeRegionsList> > result_parts;

      for (const auto& chromosome_regions : regions_data) {
        auto t = std::async(std::launch::async, &disjoin_regions,
                            std::move(chromosome_regions.second), std::ref(chromosome_regions.first));

        threads.emplace_back(std::move(t));
      }

      ChromosomeRegionsList disjoin_set;
      for (size_t i = 0; i < threads.size(); ++i) {
        threads[i].wait();
        auto result = threads[i].get();
        disjoin_set.emplace_back(std::move(result));
      }

      return disjoin_set;
    }
  }
}
