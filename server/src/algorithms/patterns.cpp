//
//  patterns.cpp
//  epidb
//
//  Created by Felipe Albrecht on 30.04.13.
//  Copyright (c) 2014 Max Planck Institute for Computer Science. All rights reserved.
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
      non_overlap_localizations.push_back(
        build_simple_region(what.position(), what.position() + what.str().size(), DATASET_EMPTY_ID)
      );
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

      return true;
    }

    Regions PatternFinder::non_overlap_regions()
    {
      non_overlap_pattern_location(sequence, pattern);
      return std::move(non_overlap_localizations);
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
      int pos = 0;
      while (boost::regex_search(start, end, what, expression, flags)) {
        overlap_localizations.push_back(
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
