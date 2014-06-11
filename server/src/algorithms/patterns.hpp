//
//  patterns.cpp
//  epidb
//
//  Created by Felipe Albrecht on 30.04.13.
//  Copyright (c) 2014 Max Planck Institute for Computer Science. All rights reserved.
//

#ifndef PATTERNS_HPP_
#define PATTERNS_HPP_

#include <string>
#include <vector>

#include <boost/regex.hpp>

#include "../regions.hpp"

namespace epidb {
  namespace algorithms {

    class PatternFinder {
    private:
      const std::string &sequence;
      const std::string &pattern;

      bool non_overlap_processed;
      bool overlap_processed;

      Regions non_overlap_localizations;
      Regions overlap_localizations;

      void non_overlap_regex_callback(const boost::match_results<std::string::const_iterator> &what);

      bool non_overlap_pattern_location(const std::string &chromosome, const std::string &pattern);
      bool overlap_pattern_location(const std::string &chromosome, const std::string &pattern);

    public:
      PatternFinder(const std::string &s, const std::string &p) :
        sequence(s), pattern(p), non_overlap_processed(false), overlap_processed(false) {}

      Regions non_overlap_regions();
      Regions overlap_regions();

    };
  }
}

#endif