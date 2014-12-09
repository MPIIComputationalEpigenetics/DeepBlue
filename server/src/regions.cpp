//
//  regions.cpp
//  epidb
//
//  Created by Felipe Albrecht on 16.04.13.
//  Copyright (c) 2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include <limits>
#include <map>
#include <string>
#include <vector>

#include <boost/foreach.hpp>
#include <boost/shared_ptr.hpp>

#include "regions.hpp"
#include "extras/utils.hpp"

namespace epidb {

  DatasetId DATASET_EMPTY_ID = std::numeric_limits<DatasetId>::min();

  Regions build_regions(size_t s)
  {
    boost::shared_ptr<std::vector<Region> > v =  boost::shared_ptr<std::vector<Region> >( new std::vector<Region>() );
    v->reserve(s);
    return v;
  }

  Regions build_regions()
  {
    return boost::shared_ptr<std::vector<Region> >( new std::vector<Region>() );
  }

  static const std::string empty_string = "";

  Length Region::length() const
  {
    return _end - _start;
  }

  DatasetId Region::dataset_id() const
  {
    return _dataset_id;
  }

  Position Region::start() const
  {
    return _start;
  }

  Position Region::end() const
  {
    return _end;
  }

  void Region::set_start(Position s)
  {
    _start = s;
  }

  void Region::set_end(Position e)
  {
    _end = e;
  }

  void Region::insert(const std::string &value)
  {
    _string_data.push_back(value);
  }

  void Region::insert(std::string &&value)
  {
    _string_data.push_back(std::move(value));
  }

  void Region::insert(const Score value)
  {
    _numeric_data.push_back(value);
  }

  void Region::insert(const int value)
  {
    _numeric_data.push_back(value);
  }

  const std::string &Region::get(const size_t pos) const
  {
    if (pos >= _string_data.size()) {
      return empty_string;
    }

    return _string_data[pos];
  }

  Score Region::value(const int pos) const
  {
    if (pos >= static_cast<int>(_numeric_data.size()) || pos < 0) {
      return std::numeric_limits<Score>::min();
    }
    return _numeric_data[pos];
  }

  bool Region::has_stats() const
  {
    return _stats_value.get();
  }

  Score Region::min() const
  {
    if (_stats_value) {
      return _stats_value->_min;
    }
    return std::numeric_limits<Score>::min();
  }

  Score Region::max() const
  {
    if (_stats_value) {
      return _stats_value->_max;
    }
    return std::numeric_limits<Score>::min();
  }

  Score Region::median() const
  {
    if (_stats_value) {
      return _stats_value->_median;
    }
    return std::numeric_limits<Score>::min();
  }

  Score Region::mean() const
  {
    if (_stats_value) {
      return _stats_value->_mean;
    }
    return std::numeric_limits<Score>::min();
  }

  Score Region::var() const
  {
    if (_stats_value) {
      return _stats_value->_var;
    }
    return std::numeric_limits<Score>::min();
  }

  Score Region::sd() const
  {
    if (_stats_value) {
      return _stats_value->_sd;
    }
    return std::numeric_limits<Score>::min();
  }

  Score Region::count() const
  {
    if (_stats_value) {
      return _stats_value->_count;
    }
    return std::numeric_limits<Score>::min();
  }
} // namespace epidb
