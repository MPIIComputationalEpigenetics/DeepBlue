//
//  accumulator.hpp
//  epidb
//
//  Created by Felipe Albrecht on 11.11.14.
//  Copyright (c) 2014 Max Planck Institute for Computer Science. All rights reserved.
//

#ifndef EPIDB_ALGORITHMS_ACCUMULATOR_HPP
#define EPIDB_ALGORITHMS_ACCUMULATOR_HPP

namespace epidb {
  namespace algorithms {

    class Accumulator {
    private:
      std::vector<double> values;
      bool _calculated;
      double _min;
      double _max;
      double _median;
      double _mean;
      double _var;
      double _sd;

    public:
      Accumulator();
      void push(double value);
      double min();
      double max();
      double mean();
      double var();
      double sd();
      double median();
      double count();
      const std::string string(std::string sep);
      void calculate();
    };
  }
}

#endif