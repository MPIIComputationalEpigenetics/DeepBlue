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

  DatasetId DATASET_EMPTY_ID = std::numeric_limits<long long>::min();

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

  void Region::set(const std::string &key, const std::string &value)
  {
    std::pair<std::string, std::string> p(key, value);
    _data.push_back(p);
  }

  const std::string &Region::get(const std::string &key) const
  {
    for (std::vector<std::pair<std::string, std::string> >::const_iterator it = _data.begin(); it != _data.end(); it++) {
      if (it->first == key) {
        return it->second;
      }
    }
    return empty_string;
  }

  Score Region::value(const std::string &key) const
  {
    for (std::vector<std::pair<std::string, std::string> >::const_iterator it = _data.begin(); it != _data.end(); it++) {
      if (it->first == key) {
        Score v;
        utils::string_to_score(it->second, v);
        return v;
      }
    }
    return 0.0;
  }

  bool Region::has_stats() const
  {
    return _stats_value;
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
