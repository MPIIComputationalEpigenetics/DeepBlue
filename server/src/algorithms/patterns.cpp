//
//  patterns.cpp
//  epidb
//
//  Created by Felipe Albrecht on 30.04.13.
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
#include <iostream>

#include <boost/bind.hpp>
#include <boost/ref.hpp>
#include <boost/regex.hpp>

#include "patterns.hpp"


namespace epidb {
  namespace algorithms {
    void PatternFinder::non_overlap_regex_callback(const boost::match_results<std::string::const_iterator> &what)
    {
      non_overlap_localizations.emplace_back(
        build_simple_region(what.position(), what.position() + what.str().size(), DATASET_EMPTY_ID)
      );
    }

    bool PatternFinder::non_overlap_pattern_location(const std::string &chromosome, const std::string &pattern)
    {
      non_overlap_localizations = Regions();
      boost::regex expression(pattern);
      boost::sregex_iterator m1(chromosome.begin(), chromosome.end(), expression);
      boost::sregex_iterator m2;

      std::for_each(m1, m2,
                    boost::bind(&PatternFinder::non_overlap_regex_callback, boost::ref(*this), _1)
                   );

      return true;
    }

    Regions PatternFinder::non_overlap_regions()
    {
      non_overlap_pattern_location(sequence, pattern);
      return std::move(non_overlap_localizations);
    }

    bool PatternFinder::overlap_pattern_location(const std::string &chromosome, const std::string &pattern)
    {
      overlap_localizations = Regions();
      std::string::const_iterator start, end;
      start = chromosome.begin();
      end = chromosome.end();
      boost::regex expression(pattern);
      boost::match_results<std::string::const_iterator> what;
      boost::match_flag_type flags = boost::match_default;
      int pos = 0;
      while (boost::regex_search(start, end, what, expression, flags)) {
        overlap_localizations.emplace_back(
          build_simple_region(pos, pos + what.str().size(), DATASET_EMPTY_ID)
        );
        start = what[0].first + 1;
        pos += (what.position() + 1);
        flags |= boost::match_prev_avail;
        flags |= boost::match_not_bob;
      }

      return true;
    }

    Regions PatternFinder::overlap_regions()
    {
      overlap_pattern_location(sequence, pattern);
      return std::move(overlap_localizations);
    }
  }
}
