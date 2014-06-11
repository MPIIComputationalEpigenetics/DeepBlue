//
//  regions.hpp
//  epidb
//
//  Created by Felipe Albrecht on 13.06.13.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#ifndef EPIDB_REGIONS_HPP
#define EPIDB_REGIONS_HPP

#include <limits>
#include <map>
#include <string>
#include <vector>

#include <boost/foreach.hpp>
#include <boost/shared_ptr.hpp>

#include "extras/utils.hpp"

namespace epidb {

  typedef boost::shared_ptr<std::string> CollectionId;

  CollectionId build_collection_id(const std::string& name);

  extern CollectionId EMPTY_COLLECTION_ID;

  struct StatsValue {
    StatsValue(double min, double max, double median, double mean, double var, double sd, double count) :
      _min(min), _max(max), _median(median), _mean(mean), _var(var), _sd(sd), _count(count) {}

    double _min;
    double _max;
    double _median;
    double _mean;
    double _var;
    double _sd;
    double _count;
  };

  class Region {
  private:
    static const std::string name;

    boost::shared_ptr<StatsValue> stats_value;
    std::map<std::string, std::string> data;

  public:
    Region() : start(-1), end(-1) {}

    Region(long long s, long long e, CollectionId _id)
      : start(s), end(e), collection_id(_id) {}

    Region(long long s, long long e, CollectionId _id,
           double min, double max, double median, double mean, double var, double sd, double count) :
      stats_value(new StatsValue(min, max, median, mean, var, sd, count)),
      start(s), end(e), collection_id(_id)
    { }

    size_t start;
    size_t end;
    CollectionId collection_id;

    bool operator<(const Region &other) const
    {
      return start < other.start;
    }

    size_t length();
    void set(const std::string &key, const std::string &value);
    const std::string  &get(const std::string &key) const;
    double value(const std::string &key) const;

    bool has_stats() const;
    double min() const;
    double max() const;
    double median() const;
    double mean() const;
    double var() const;
    double sd() const;
    double count() const;

  };

  typedef boost::shared_ptr<std::vector<Region> > Regions;
  typedef std::vector<Region>::iterator RegionsIterator;
  typedef std::vector<Region>::const_iterator RegionsConstIterator;

  Regions build_regions();

  typedef std::pair<std::string, Regions> ChromosomeRegions;
  // TODO: to class/struct.
  // TOOD: ::insert(chr, data)
  // TOOD: ::get(chr)
  typedef std::vector<ChromosomeRegions> ChromosomeRegionsList;

} // namespace epidb

#endif