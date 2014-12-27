//
//  regions.hpp
//  epidb
//
//  Created by Felipe Albrecht on 18.12.14.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#ifndef EPIDB_REGIONS_HPP
#define EPIDB_REGIONS_HPP

#include <limits>
#include <string>
#include <vector>

#include "../types.hpp"

namespace epidb {

  typedef int DatasetId;

  extern DatasetId DATASET_EMPTY_ID;

  class AbstractRegion {
  private:
    DatasetId _dataset_id;
    Position _start;
    Position _end;

  public:
    AbstractRegion() :
      _dataset_id(),
      _start(-1),
      _end(-1) {}

    AbstractRegion(Position s, Position e, DatasetId _id):
      _dataset_id(_id),
      _start(s),
      _end(e) {}


    // The value returned indicates whether the element passed as first argument is considered to go before the second in the specific strict weak ordering it defines.
    bool operator<(const AbstractRegion &other) const
    {
      if (_start < other._start) {
        return true;
      }

      if (other._start < _start) {
        return false;
      }

      return _end < other._end;
    }

    DatasetId dataset_id() const;
    Length length() const;
    Position start() const;
    Position end() const;
    void set_start(Position s);
    void set_end(Position e);
    virtual void insert(const std::string &value);
    virtual void insert(std::string &&value);
    virtual void insert(const float value);
    virtual void insert(const int value);
    virtual const std::string  &get_string(const size_t pos) const;
    virtual Score value(const size_t pos) const;
    virtual bool has_stats() const;
    virtual const AbstractRegion &ref() const;
  };


  // -----------------------------------
  // SimpleRegion
  // -----------------------------------
  class SimpleRegion : public AbstractRegion  {

  public:
    SimpleRegion(Position s, Position e, DatasetId _id):
      AbstractRegion(s, e, _id) {}
  };


  // -----------------------------------
  // BedRegion
  // -----------------------------------
  class BedRegion : public AbstractRegion {
    std::vector<std::string> _string_data;
    std::vector<float> _numeric_data;

  public:
    BedRegion(Position s, Position e, DatasetId _id):
      AbstractRegion(s, e, _id) {}

    virtual void insert(const std::string &value);
    virtual void insert(std::string &&value);
    virtual void insert(const float value);
    virtual void insert(const int value);
    virtual const std::string  &get_string(const size_t pos) const;
    virtual Score value(const size_t pos) const;
  };


  // -----------------------------------
  // WigRegion
  // -----------------------------------
  class WigRegion : public AbstractRegion  {
  private:
    Score _value;

  public:
    WigRegion(Position s, Position e, DatasetId _id, Score value):
      AbstractRegion(s, e, _id),
      _value(value) {}

    virtual Score value(const size_t pos) const;
  };


  // -----------------------------------
  // AggregateRegion
  // -----------------------------------
  class AggregateRegion : public AbstractRegion {
  private:
    Score _min;
    Score _max;
    Score _median;
    Score _mean;
    Score _var;
    Score _sd;
    Score _count;

  public:
    AggregateRegion(Position s, Position e, DatasetId _id, Score min, Score max, Score median, Score mean, Score var, Score sd, Score count) :
      AbstractRegion(s, e, _id),
      _min(min),
      _max(max),
      _median(median),
      _mean(mean),
      _var(var),
      _sd(sd),
      _count(count) {}

    bool has_stats() const;
    Score min() const;
    Score max() const;
    Score median() const;
    Score mean() const;
    Score var() const;
    Score sd() const;
    Score count() const;
  };


  typedef std::unique_ptr<AbstractRegion> RegionPtr;
  typedef std::vector<RegionPtr> Regions;

  Regions build_regions();
  Regions build_regions(size_t s);

  RegionPtr build_simple_region(Position s, Position e, DatasetId _id);
  RegionPtr build_bed_region(Position s, Position e, DatasetId _id);
  RegionPtr build_wig_region(Position s, Position e, DatasetId _id, Score value);
  RegionPtr build_aggregte_region(Position s, Position e, DatasetId _id, Score min, Score max, Score median, Score mean, Score var, Score sd, Score count);

  typedef std::pair<std::string, Regions> ChromosomeRegions;
  typedef std::vector<ChromosomeRegions> ChromosomeRegionsList;

} // namespace epidb

#endif