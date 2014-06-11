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

  Regions build_regions()
  {
    return boost::shared_ptr<std::vector<Region> >( new std::vector<Region> );
  }

  static const std::string empty_string = "";

  size_t Region::length()
  {
    return end - start;
  }

  void Region::set(const std::string &key, const std::string &value)
  {
    data[key] = value;
  }

  const std::string &Region::get(const std::string &key) const
  {
    std::map<std::string, std::string>::const_iterator it = data.find(key);
    if (it == data.end()) {
      return empty_string;
    }
    return it->second;
  }

  double Region::value(const std::string &key) const
  {
    std::map<std::string, std::string>::const_iterator it = data.find(key);
    if (it == data.end()) {
      return 0.0;
    }
    double v;
    utils::string_to_double(it->second, v);
    return v;
  }

  bool Region::has_stats() const
  {
    return stats_value;
  }

  double Region::min() const
  {
    if (stats_value) {
      return stats_value->_min;
    }
    return std::numeric_limits<double>::min();
  }

  double Region::max() const
  {
    if (stats_value) {
      return stats_value->_max;
    }
    return std::numeric_limits<double>::min();
  }

  double Region::median() const
  {
    if (stats_value) {
      return stats_value->_median;
    }
    return std::numeric_limits<double>::min();
  }

  double Region::mean() const
  {
    if (stats_value) {
      return stats_value->_mean;
    }
    return std::numeric_limits<double>::min();
  }

  double Region::var() const
  {
    if (stats_value) {
      return stats_value->_var;
    }
    return std::numeric_limits<double>::min();
  }

  double Region::sd() const
  {
    if (stats_value) {
      return stats_value->_sd;
    }
    return std::numeric_limits<double>::min();
  }

  double Region::count() const
  {
    if (stats_value) {
      return stats_value->_count;
    }
    return std::numeric_limits<double>::min();
  }
} // namespace epidb
