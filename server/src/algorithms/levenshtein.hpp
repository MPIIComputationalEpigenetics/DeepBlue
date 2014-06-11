//
//  levenshtein.hpp
//  epidb
//
//  Created by Felipe Albrecht on 26.06.13.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
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
