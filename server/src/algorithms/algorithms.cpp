//
//  algorithms.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 23.06.17.
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

#include <vector>
#include <set>
#include <iostream>
#include <unordered_map>

#include "../datatypes/regions.hpp"
#include "../processing/processing.hpp"

namespace epidb {
  namespace algorithms {

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

    Length calculate_distance(const RegionPtr& r1, const RegionPtr& r2)
    {
      return r2->start() - r1->end();
    }

    Length calculate_overlap(const RegionPtr& r1, const RegionPtr& r2)
    {
      return r1->end() - r2->start();
    }

  }
}