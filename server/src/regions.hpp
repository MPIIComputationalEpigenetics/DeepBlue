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

#include "types.hpp"

namespace epidb {

  typedef int DatasetId;

  extern DatasetId DATASET_EMPTY_ID;

  struct StatsValue {
    StatsValue(Score min, Score max, Score median, Score mean, Score var, Score sd, Score count) :
      _min(min), _max(max), _median(median), _mean(mean), _var(var), _sd(sd), _count(count) {}

    Score _min;
    Score _max;
    Score _median;
    Score _mean;
    Score _var;
    Score _sd;
    Score _count;
  };

  class Region {
  private:
    DatasetId _dataset_id;
    Position _start;
    Position _end;
    boost::shared_ptr<StatsValue> _stats_value;
    std::vector<std::pair<std::string, std::string> > _data;

  public:
    Region() :
      _dataset_id(),
      _start(-1),
      _end(-1),
      _stats_value(),
      _data() {}

    Region(Position s, Position e, DatasetId _id):
      _dataset_id(_id),
      _start(s),
      _end(e),
      _stats_value(),
      _data() {}

    Region(Position s, Position e, DatasetId _id,
           Score min, Score max, Score median, Score mean, Score var, Score sd, Score count) :
      _dataset_id(_id),
      _start(s),
      _end(e),
      _stats_value(new StatsValue(min, max, median, mean, var, sd, count)),
      _data() {}

    bool operator<(const Region &other) const
    {
      return _start < other._start;
    }

    DatasetId dataset_id() const;
    Length length() const;
    Position start() const;
    Position end() const;
    void set_start(Position s);
    void set_end(Position e);
    void set(const std::string &key, const std::string &value);
    const std::string  &get(const std::string &key) const;
    Score value(const std::string &key) const;

    bool has_stats() const;
    Score min() const;
    Score max() const;
    Score median() const;
    Score mean() const;
    Score var() const;
    Score sd() const;
    Score count() const;

  };

  typedef boost::shared_ptr<std::vector<Region> > Regions;
  typedef std::vector<Region>::iterator RegionsIterator;
  typedef std::vector<Region>::const_iterator RegionsConstIterator;

  Regions build_regions();
  Regions build_regions(size_t s);

  typedef std::pair<std::string, Regions> ChromosomeRegions;
  typedef std::vector<ChromosomeRegions> ChromosomeRegionsList;

} // namespace epidb

#endif