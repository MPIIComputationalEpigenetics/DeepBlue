//
//  regions.cpp
//  epidb
//
//  Created by Felipe Albrecht on 18.12.14.
//  Copyright (c) 2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include <limits>
#include <memory>
#include <string>
#include <vector>

#include "../types.hpp"

#include "regions.hpp"

namespace epidb {

  DatasetId DATASET_EMPTY_ID = std::numeric_limits<DatasetId>::min();

  Regions build_regions()
  {
    return std::vector<RegionPtr>();
  }

  Regions build_regions(size_t s)
  {
    Regions v =  build_regions();
    v.reserve(s);
    return v;
  }

  static const std::string empty_string = "";

  Length AbstractRegion::length() const
  {
    return _end - _start;
  }

  DatasetId AbstractRegion::dataset_id() const
  {
    return _dataset_id;
  }

  Position AbstractRegion::start() const
  {
    return _start;
  }

  Position AbstractRegion::end() const
  {
    return _end;
  }

  void AbstractRegion::set_start(Position s)
  {
    _start = s;
  }

  void AbstractRegion::set_end(Position e)
  {
    _end = e;
  }

  bool AbstractRegion::has_stats() const
  {
    return false;
  }

  void AbstractRegion::insert(const std::string &value)
  {
    // Nothing
  }

  void AbstractRegion::insert(std::string &&value)
  {
    // Nothing
  }

  void AbstractRegion::insert(const Score value)
  {
    // Nothing
  }

  void AbstractRegion::insert(const int value)
  {
    // Nothing
  }

  size_t AbstractRegion::size() const
  {
    static size_t size = sizeof(AbstractRegion) + sizeof(void *);
    return size;
  }

  Score AbstractRegion::value(const size_t pos) const
  {
    return std::numeric_limits<Score>::min();
  }

  static std::string EMPTY_STRING;
  const std::string AbstractRegion::attribute(const std::string key) const
  {
    return EMPTY_STRING;
  }

  const std::string &AbstractRegion::get_string(const size_t pos) const
  {
    return empty_string;
  }

  // -----------------------------------
  // SimpleRegion
  // -----------------------------------
  RegionPtr SimpleRegion::clone() const
  {
    return  RegionPtr(new SimpleRegion(*this));
  }

  size_t SimpleRegion::size() const
  {
    static size_t size = sizeof(SimpleRegion) + sizeof(void *);
    return  size;
  }



  // -----------------------------------
  // BedRegion
  // -----------------------------------

  void BedRegion::insert(const std::string &value)
  {
    _string_data.push_back(value);
  }

  void BedRegion::insert(std::string &&value)
  {
    _string_data.push_back(std::move(value));
  }

  void BedRegion::insert(const Score value)
  {
    _numeric_data.push_back(value);
  }

  void BedRegion::insert(const int value)
  {
    _numeric_data.push_back(value);
  }

  Score BedRegion::value(const size_t pos) const
  {
    if (pos >= _numeric_data.size()) {
      return std::numeric_limits<Score>::min();
    }
    return _numeric_data[pos];
  }

  const std::string &BedRegion::get_string(const size_t pos) const
  {
    if (pos >= _string_data.size()) {
      return empty_string;
    }

    return _string_data[pos];
  }

  size_t BedRegion::size() const
  {
    static size_t pre_size = sizeof(BedRegion) + sizeof(void *);
    size_t size = pre_size + (_numeric_data.size() * sizeof(int));
    for (const auto& s : _string_data) {
      size += s.capacity();
    }
    return size;
  }

  RegionPtr BedRegion::clone() const
  {
    return RegionPtr(new BedRegion(*this));
  }


  // -----------------------------------
  // WigRegion
  // -----------------------------------
  Score WigRegion::value(const size_t pos) const
  {
    if (pos > 0) {
      return std::numeric_limits<Score>::min();
    }
    return _value;
  }

  size_t WigRegion::size() const
  {
    static size_t size = sizeof(WigRegion) + sizeof(void *);
    return size;
  }

  RegionPtr WigRegion::clone() const
  {
    return RegionPtr(new WigRegion(*this));
  }


  // -----------------------------------
  // GeneRegion
  // -----------------------------------
  const std::string GeneRegion::attribute(const std::string key) const
  {
    auto it = _attributes.find(key);
    if (it == _attributes.end()) {
      return EMPTY_STRING;
    }
    return it->second;
  }

  size_t GeneRegion::size() const
  {
    static size_t size = sizeof(WigRegion) + sizeof(_attributes) + + sizeof(void *);
    return size;
  }

  RegionPtr GeneRegion::clone() const
  {
    return RegionPtr(new GeneRegion(*this));
  }

  // -----------------------------------
  // AggregateRegion
  // -----------------------------------
  bool AggregateRegion::has_stats() const
  {
    return true;
  }

  Score AggregateRegion::min() const
  {
    return _min;
  }

  Score AggregateRegion::max() const
  {
    return _max;
  }

  Score AggregateRegion::median() const
  {
    return _median;
  }

  Score AggregateRegion::mean() const
  {
    return _mean;
  }

  Score AggregateRegion::var() const
  {
    return _var;
  }

  Score AggregateRegion::sd() const
  {
    return _sd;
  }

  Score AggregateRegion::count() const
  {
    return _count;
  }

  size_t AggregateRegion::size() const
  {
    static size_t size = sizeof(AggregateRegion) + sizeof(void *);
    return size;
  }

  RegionPtr AggregateRegion::clone() const
  {
    return RegionPtr(new AggregateRegion(*this));
  }

  // ------------------------
  // Builders
  // ------------------------

  RegionPtr build_simple_region(Position s, Position e, DatasetId _id)
  {
    return std::unique_ptr<SimpleRegion>(new SimpleRegion(s, e, _id));
  }

  RegionPtr build_bed_region(Position s, Position e, DatasetId _id)
  {
    return std::unique_ptr<BedRegion>(new BedRegion(s, e, _id));
  }

  RegionPtr build_wig_region(Position s, Position e, DatasetId _id, Score value)
  {
    return std::unique_ptr<WigRegion>(new WigRegion(s, e, _id, value));
  }

  RegionPtr build_gene_region(Position s, Position e, DatasetId _id, std::string _source, Score _score, char _strand, char _frame, datatypes::Metadata& attributes)
  {
    return std::unique_ptr<GeneRegion>(new GeneRegion(s, e, _id, _source, _score, _strand, _frame, attributes));
  }

  RegionPtr build_aggregte_region(Position s, Position e, DatasetId _id, Score min, Score max, Score median, Score mean, Score var, Score sd, Score count)
  {
    return std::unique_ptr<AggregateRegion>(new AggregateRegion(s, e, _id, min, max, median, mean, var, sd, count));
  }
} // namespace epidb
