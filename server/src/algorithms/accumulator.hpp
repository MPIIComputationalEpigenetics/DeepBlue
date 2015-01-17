//
//  accumulator.hpp
//  epidb
//
//  Created by Felipe Albrecht on 11.11.14.
//  Copyright (c) 2014 Max Planck Institute for Computer Science. All rights reserved.
//

#ifndef EPIDB_ALGORITHMS_ACCUMULATOR_HPP
#define EPIDB_ALGORITHMS_ACCUMULATOR_HPP

#include <string>

#include "../types.hpp"

namespace epidb {
  namespace algorithms {

    class Accumulator {
    private:
      std::vector<Score> values;
      bool _calculated;
      Score _min;
      Score _max;
      Score _median;
      Score _mean;
      Score _var;
      Score _sd;

      void calculate();

    public:
      Accumulator();
      void push(Score value);
      Score min();
      Score max();
      Score mean();
      Score var();
      Score sd();
      Score median();
      Score count();
      const std::string string(std::string sep);
    };

    typedef Score (Accumulator::*GetDataPtr)();
    GetDataPtr get_function_data(const std::string& function_name);
  }
}

#endif
