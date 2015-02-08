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
      mutable bool _calculated;

      mutable Score _min;
      mutable Score _max;
      mutable Score _median;
      mutable Score _mean;
      mutable Score _var;
      mutable Score _sd;

      void calculate() const;

    public:
      Accumulator();
      void push(Score value);
      Score min() const;
      Score max() const;
      Score mean() const;
      Score var() const;
      Score sd() const;
      Score median() const;
      Score count() const;
      const std::string string(std::string sep) const;
    };

    typedef Score (Accumulator::*GetDataPtr)() const;
    GetDataPtr get_function_data(const std::string& function_name);
  }
}

#endif
