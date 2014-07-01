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
#include <memory>
#include <string>
#include <vector>

#include <boost/foreach.hpp>
#include <boost/shared_ptr.hpp>

#include "extras/utils.hpp"

namespace epidb {

  typedef boost::shared_ptr<std::string> CollectionId;

  CollectionId build_collection_id(const std::string &name);

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
    CollectionId _collection_id; // Reference
    size_t _start;
    size_t _end;
    boost::shared_ptr<StatsValue> _stats_value;
    std::vector<std::pair<std::string, std::string> > _data;

  public:
    Region() :
      _collection_id(),
      _start(-1),
      _end(-1),
      _stats_value(),
      _data() {}


    Region(CollectionId _id) :
      _collection_id(_id),
      _start(-1),
      _end(-1),
      _stats_value(),
      _data() {}

    Region(long long s, long long e, CollectionId _id):
      _collection_id(_id),
      _start(s),
      _end(e),
      _stats_value(),
      _data() {}

    Region(long long s, long long e, CollectionId _id,
           double min, double max, double median, double mean, double var, double sd, double count) :
      _collection_id(_id),
      _start(s),
      _end(e),
      _stats_value(new StatsValue(min, max, median, mean, var, sd, count)),
      _data() {}

    bool operator<(const Region &other) const
    {
      return _start < other._start;
    }

    CollectionId collection_id() const;
    size_t length() const;
    size_t start() const;
    size_t end() const;
    void set_start(size_t s);
    void set_end(size_t e);
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
  typedef std::vector<ChromosomeRegions> ChromosomeRegionsList;

} // namespace epidb

#endif