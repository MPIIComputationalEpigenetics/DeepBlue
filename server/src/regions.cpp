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

  CollectionId build_collection_id(const std::string &name)
  {
    return boost::shared_ptr<std::string>(new std::string(name));
  }

  CollectionId EMPTY_COLLECTION_ID = build_collection_id("");

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

  size_t Region::length() const
  {
    return _end - _start;
  }

  CollectionId Region::collection_id() const
  {
    return _collection_id;
  }

  size_t Region::start() const
  {
    return _start;
  }

  size_t Region::end() const
  {
    return _end;
  }

  void Region::set_start(size_t s)
  {
    _start = s;
  }

  void Region::set_end(size_t e)
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

  double Region::value(const std::string &key) const
  {
    for (std::vector<std::pair<std::string, std::string> >::const_iterator it = _data.begin(); it != _data.end(); it++) {
      if (it->first == key) {
        double v;
        utils::string_to_double(it->second, v);
        return v;
      }
    }
    return 0.0;
  }

  bool Region::has_stats() const
  {
    return _stats_value;
  }

  double Region::min() const
  {
    if (_stats_value) {
      return _stats_value->_min;
    }
    return std::numeric_limits<double>::min();
  }

  double Region::max() const
  {
    if (_stats_value) {
      return _stats_value->_max;
    }
    return std::numeric_limits<double>::min();
  }

  double Region::median() const
  {
    if (_stats_value) {
      return _stats_value->_median;
    }
    return std::numeric_limits<double>::min();
  }

  double Region::mean() const
  {
    if (_stats_value) {
      return _stats_value->_mean;
    }
    return std::numeric_limits<double>::min();
  }

  double Region::var() const
  {
    if (_stats_value) {
      return _stats_value->_var;
    }
    return std::numeric_limits<double>::min();
  }

  double Region::sd() const
  {
    if (_stats_value) {
      return _stats_value->_sd;
    }
    return std::numeric_limits<double>::min();
  }

  double Region::count() const
  {
    if (_stats_value) {
      return _stats_value->_count;
    }
    return std::numeric_limits<double>::min();
  }
} // namespace epidb
