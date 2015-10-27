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

#include "../types.hpp"

namespace epidb {
  namespace algorithms {
    Accumulator::Accumulator() : _calculated(false),
      _min(0.0),
      _max(0.0),
      _median(0.0),
      _mean(0.0),
      _var(0.0),
      _sd(0.0) {}

    void Accumulator::push(Score value)
    {
      values.push_back(value);
      _calculated = false;
    }

    Score Accumulator::min() const
    {
      calculate();
      return _min;
    }

    Score Accumulator::max() const
    {
      calculate();
      return _max;
    }

    Score Accumulator::mean() const
    {
      calculate();
      return _mean;
    }

    Score Accumulator::var() const
    {
      calculate();
      return _var;
    }

    Score Accumulator::sd() const
    {
      calculate();
      return _sd;
    }

    Score Accumulator::median() const
    {
      calculate();
      return _median;
    }

    Score Accumulator::count() const
    {
      return values.size();
    }

    Score Accumulator::boolean() const
    {
      return !values.empty();
    }

    void Accumulator::calculate() const
    {
      std::vector<Score> calculated_values = values;

      if (_calculated || values.empty()) {
        return;
      }

      std::sort(calculated_values.begin(), calculated_values.end());

      _min = calculated_values[0];
      _max = calculated_values[calculated_values.size() - 1];
      _median = calculated_values[calculated_values.size() / 2];

      Score sum = std::accumulate(calculated_values.begin(), calculated_values.end(), 0.0);
      _mean = sum / calculated_values.size();

      std::vector<Score> diff(calculated_values.size());
      std::transform(calculated_values.begin(), calculated_values.end(), diff.begin(),
                     std::bind2nd(std::minus<Score>(), _mean));
      Score sq_sum = std::inner_product(diff.begin(), diff.end(), diff.begin(), 0.0);
      _var = sq_sum / calculated_values.size();
      _sd = std::sqrt(_var);

      _calculated = true;
    }

    const std::string Accumulator::string(std::string sep) const
    {
      if (values.empty()) {
        return std::string();
      }
      return utils::vector_to_string(values, sep);
    }

    GetDataPtr get_function_data(const std::string& function_name) {
      if (function_name == "min") {
        return &Accumulator::min;
      }

      if (function_name == "max") {
        return &Accumulator::max;
      }

      if (function_name == "mean") {
        return &Accumulator::mean;
      }

      if (function_name == "var") {
        return &Accumulator::var;
      }

      if (function_name == "sd") {
        return &Accumulator::sd;
      }

      if (function_name == "median") {
        return &Accumulator::median;
      }

      if (function_name == "count") {
        return &Accumulator::count;
      }

      if (function_name == "boolean") {
        return &Accumulator::boolean;
      }

      return nullptr;
    }
  }
}