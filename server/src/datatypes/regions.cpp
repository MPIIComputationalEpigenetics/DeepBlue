//
//  regions.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 18.12.14.
//  Copyright (c) 2016 Max Planck Institute for Informatics. All rights reserved.

//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.

//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.

//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#include <limits>
#include <memory>
#include <string>
#include <vector>

#include "../types.hpp"
#include "../extras/utils.hpp"

#include "regions.hpp"

namespace epidb {

  DatasetId DATASET_EMPTY_ID = std::numeric_limits<DatasetId>::min();

  static const std::string empty_string("");

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

  bool AbstractRegion::has_strand() const
  {
    return false;
  }

  const std::string AbstractRegion::strand() const
  {
    return empty_string;
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

  static const datatypes::Metadata EMPTY_ATTRIBUTES;
  const datatypes::Metadata& AbstractRegion::attributes() const
  {
    return EMPTY_ATTRIBUTES;
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
  // StrandedRegion
  // -----------------------------------
  bool StrandedRegion::has_strand() const
  {
    return true;
  }

  const std::string StrandedRegion::strand() const
  {
    return _strand;
  }

  size_t StrandedRegion::size() const
  {
    return BedRegion::size() + _strand.capacity();
  }

  RegionPtr StrandedRegion::clone() const
  {
    return RegionPtr(new StrandedRegion(*this));
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
  bool GeneRegion::has_strand() const
  {
    return true;
  }

  const std::string GeneRegion::strand() const
  {
    return _strand;
  }

  const datatypes::Metadata& GeneRegion::attributes() const
  {
    return _attributes;
  }

  const std::string &GeneRegion::get_string(const size_t pos) const
  {
    switch (pos) {
    case 0: return _source;
    case 1: return _feature;
    case 2: {
      if (_score_cache.empty()) {
        if (_score == std::numeric_limits<Score>::min()) {
          _score_cache = ".";
        } else {
          _score_cache = utils::score_to_string(_score);
        }
      }
      return _score_cache;
    }
    case 3: return _strand;
    case 4: return _frame;
    case 5: {
      if (_attributes_cache.empty()) {
        bool first = true;
        std::stringstream ss;
        for (auto &kv : _attributes) {
          if (first) {
            first = false;
          } else {
            ss << "; ";
            first = false;
          }
          ss << kv.first << " \"" << kv.second << "\"";
        }
        _attributes_cache = ss.str();
      }
      return _attributes_cache;
    }
    default: return empty_string;
    }
  }

  size_t GeneRegion::size() const
  {
    size_t attributes_size = 0;
    for (auto attribute : _attributes) {
      attributes_size += attribute.first.capacity();
      attributes_size += attribute.second.capacity();
    }

    static size_t size = sizeof(WigRegion) + attributes_size + + sizeof(void *);
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

  Score AggregateRegion::sum() const
  {
    return _sum;
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

  RegionPtr build_stranded_region(Position s, Position e, DatasetId _id, std::string strand)
  {
    return std::unique_ptr<StrandedRegion>(new StrandedRegion(s, e, _id, strand));
  }

  RegionPtr build_wig_region(Position s, Position e, DatasetId _id, Score value)
  {
    return std::unique_ptr<WigRegion>(new WigRegion(s, e, _id, value));
  }

  RegionPtr build_gene_region(Position s, Position e, DatasetId _id, std::string source, Score score, std::string feature, std::string strand, std::string frame, datatypes::Metadata& attributes)
  {
    return std::unique_ptr<GeneRegion>(new GeneRegion(s, e, _id, source, score, feature, strand, frame, attributes));
  }

  RegionPtr build_aggregte_region(Position s, Position e, DatasetId _id, Score min, Score max, Score sum, Score median, Score mean, Score var, Score sd, Score count)
  {
    return std::unique_ptr<AggregateRegion>(new AggregateRegion(s, e, _id, min, max, sum, median, mean, var, sd, count));
  }
} // namespace epidb
