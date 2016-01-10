//
//  patterns.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 30.04.13.
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

#ifndef PATTERNS_HPP_
#define PATTERNS_HPP_

#include <string>
#include <vector>

#include <boost/regex.hpp>

#include "../datatypes/regions.hpp"

namespace epidb {
  namespace algorithms {

    class PatternFinder {
    private:
      const std::string &sequence;
      const std::string &pattern;

      Regions non_overlap_localizations;
      Regions overlap_localizations;

      void non_overlap_regex_callback(const boost::match_results<std::string::const_iterator> &what);

      bool non_overlap_pattern_location(const std::string &chromosome, const std::string &pattern);
      bool overlap_pattern_location(const std::string &chromosome, const std::string &pattern);

    public:
      PatternFinder(const std::string &s, const std::string &p) :
        sequence(s), pattern(p) {}

      Regions non_overlap_regions();
      Regions overlap_regions();

    };
  }
}

#endif