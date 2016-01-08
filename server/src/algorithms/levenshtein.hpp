//
//  levenshtein.hpp
//  epidb
//
//  Created by Felipe Albrecht on 26.06.13.
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

#include <string>
#include <vector>

#ifndef EPIDB_ALGORITHMS_LEVENSHTEIN_HPP
#define EPIDB_ALGORITHMS_LEVENSHTEIN_HPP

namespace epidb {
  namespace algorithms {
    class Levenshtein {
    public:
      float static calculate_score(const std::string& source, const std::string& tm);
      std::vector<std::string> static order_by_score(const std::string& term, const std::vector<std::string>& tms);
    };
  }
}

#endif
