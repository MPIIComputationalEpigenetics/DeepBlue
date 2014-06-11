//
//  patterns.cpp
//  epidb
//
//  Created by Felipe Albrecht on 30.04.13.
//  Copyright (c) 2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include <string>
#include <fstream>
#include <functional>

#include <iostream>

#include <boost/foreach.hpp>
#include <boost/bind.hpp>
#include <boost/ref.hpp>
#include <boost/regex.hpp>

#include "patterns.hpp"


namespace epidb {
  namespace algorithms {
    void PatternFinder::non_overlap_regex_callback(const boost::match_results<std::string::const_iterator> &what)
    {
      Region region(what.position(), what.position() + what.str().size(), EMPTY_COLLECTION_ID);
      non_overlap_localizations->push_back(region);
    }

    bool PatternFinder::non_overlap_pattern_location(const std::string &chromosome, const std::string &pattern)
    {
      non_overlap_localizations = build_regions();
      boost::regex expression(pattern);
      boost::sregex_iterator m1(chromosome.begin(), chromosome.end(), expression);
      boost::sregex_iterator m2;

      std::for_each(m1, m2,
                    boost::bind(&PatternFinder::non_overlap_regex_callback, boost::ref(*this), _1)
                   );

      non_overlap_processed = true;
      return true;
    }

    Regions PatternFinder::non_overlap_regions()
    {
      if (!non_overlap_processed) {
        non_overlap_pattern_location(sequence, pattern);
      }
      return non_overlap_localizations;
    }

    bool PatternFinder::overlap_pattern_location(const std::string &chromosome, const std::string &pattern)
    {
      overlap_localizations = build_regions();
      std::string::const_iterator start, end;
      start = chromosome.begin();
      end = chromosome.end();
      boost::regex expression(pattern);
      boost::match_results<std::string::const_iterator> what;
      boost::match_flag_type flags = boost::match_default;
      while (boost::regex_search(start, end, what, expression, flags)) {
        Region region(what.position(), what.position() + what.str().size(), EMPTY_COLLECTION_ID);
        overlap_localizations->push_back(region);
        start = what[0].first + 1;
        flags |= boost::match_prev_avail;
        flags |= boost::match_not_bob;
      }


      return true;
    }

    Regions PatternFinder::overlap_regions()
    {
      if (!overlap_processed) {
        overlap_pattern_location(sequence, pattern);
      }
      return overlap_localizations;
    }
  }
}
