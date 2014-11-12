//
//  accumulator.cpp
//  epidb
//
//  Created by Felipe Albrecht on 11.11.14.
//  Copyright (c) 2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include <functional>
#include <cmath>
#include <numeric>
#include <vector>

#include "accumulator.hpp"
#include "../extras/utils.hpp"

namespace epidb {
  namespace algorithms {
    Accumulator::Accumulator() : _calculated(false),
      _min(0.0),
      _max(0.0),
      _median(0.0),
      _mean(0.0),
      _var(0.0),
      _sd(0.0) {}

    void Accumulator::push(double value)
    {
      values.push_back(value);
      _calculated = false;
    }

    double Accumulator::min()
    {
      calculate();
      return _min;
    }

    double Accumulator::max()
    {
      calculate();
      return _max;
    }

    double Accumulator::mean()
    {
      calculate();
      return _mean;
    }

    double Accumulator::var()
    {
      calculate();
      return _var;
    }

    double Accumulator::sd()
    {
      calculate();
      return _sd;
    }

    double Accumulator::median()
    {
      calculate();
      return _median;
    }

    double Accumulator::count()
    {
      return values.size();
    }

    void Accumulator::calculate()
    {
      std::vector<double> calculated_values = values;

      if (_calculated || values.empty()) {
        return;
      }

      std::sort(calculated_values.begin(), calculated_values.end());

      _min = calculated_values[0];
      _max = calculated_values[calculated_values.size() - 1];
      _median = calculated_values[calculated_values.size() / 2];

      double sum = std::accumulate(calculated_values.begin(), calculated_values.end(), 0.0);
      _mean = sum / calculated_values.size();

      std::vector<double> diff(calculated_values.size());
      std::transform(calculated_values.begin(), calculated_values.end(), diff.begin(),
                     std::bind2nd(std::minus<double>(), _mean));
      double sq_sum = std::inner_product(diff.begin(), diff.end(), diff.begin(), 0.0);
      _var = sq_sum / calculated_values.size();
      _sd = std::sqrt(_var);

      _calculated = true;
    }

    const std::string Accumulator::string(std::string sep)
    {
      if (values.empty()) {
        return std::string();
      }
      return utils::vector_to_string(values, sep);
    }
  }
}