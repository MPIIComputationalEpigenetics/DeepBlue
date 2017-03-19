//
//  regions.hpp
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

#ifndef EPIDB_REGIONS_HPP
#define EPIDB_REGIONS_HPP

#include <iostream>

#include <limits>
#include <memory>
#include <string>
#include <vector>

#include "../datatypes/gene_ontology_terms.hpp"
#include "../datatypes/metadata.hpp"

#include "../types.hpp"

namespace epidb {

  typedef int DatasetId;
  extern DatasetId DATASET_EMPTY_ID;

  //////////////////////////////////
  // Regions types
  //////////////////////////////////

  class AbstractRegion;
  typedef std::unique_ptr<AbstractRegion> RegionPtr;

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

    virtual ~AbstractRegion() { }

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
    virtual const std::string strand() const;
    virtual const datatypes::Metadata& attributes() const;

    virtual bool has_gene_infos() const;
    virtual bool has_stats() const;
    virtual bool has_strand() const;
    virtual size_t size() const;
    virtual RegionPtr clone() const = 0;
  };


  // -----------------------------------
  // SimpleRegion
  // -----------------------------------
  class SimpleRegion : public AbstractRegion  {

  public:
    SimpleRegion(Position s, Position e, DatasetId _id):
      AbstractRegion(s, e, _id) {}

    virtual RegionPtr clone() const;

    virtual size_t size() const;
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
    virtual size_t size() const;
    virtual RegionPtr clone() const;
  };


  // -----------------------------------
  // StrandedRegion
  // -----------------------------------
  class StrandedRegion : public BedRegion {
  private:
    std::string _strand;

  public:
    StrandedRegion(Position s, Position e, DatasetId _id, std::string d):
      BedRegion(s, e, _id),
      _strand(d) {}

    virtual bool has_strand() const;
    virtual const std::string strand() const;
    virtual size_t size() const;
    virtual RegionPtr clone() const;
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
      _value(value) { }

    virtual Score value(const size_t pos) const;
    virtual size_t size() const;
    virtual RegionPtr clone() const;
  };

  // -----------------------------------
  // GeneRegion
  // -----------------------------------
  class GeneRegion : public AbstractRegion {
    std::string _source;
    std::string _feature;
    std::string _strand;
    std::string _frame;
    datatypes::Metadata _attributes;
    mutable std::string _attributes_cache;
    Score _score;
    mutable std::string _score_cache;
    std::vector<datatypes::GeneOntologyTermPtr> _go_terms;

  public:
    GeneRegion(Position s, Position e, DatasetId _id,
               std::string source, Score score, std::string feature, std::string strand, std::string frame,
               datatypes::Metadata& attributes, std::vector<datatypes::GeneOntologyTermPtr>& go_terms):
      AbstractRegion(s, e, _id),
      _source(source),
      _feature(feature),
      _strand(strand),
      _frame(frame),
      _attributes(attributes),
      _attributes_cache(""),
      _score(score),
      _score_cache(""),
      _go_terms(go_terms)
    {}

    const std::vector<datatypes::GeneOntologyTermPtr>& get_gene_ontology_terms() const;
    virtual bool has_strand() const;
    virtual bool has_gene_infos() const;
    virtual const std::string strand() const;
    virtual const datatypes::Metadata& attributes() const;
    virtual const std::string  &get_string(const size_t pos) const;
    virtual size_t size() const;
    virtual RegionPtr clone() const;
  };

  // -----------------------------------
  // AggregateRegion
  // -----------------------------------
  class AggregateRegion : public AbstractRegion {
  private:
    Score _min;
    Score _max;
    Score _sum;
    Score _median;
    Score _mean;
    Score _var;
    Score _sd;
    Score _count;

  public:
    AggregateRegion(Position s, Position e, DatasetId _id, Score min, Score max, Score sum, Score median, Score mean, Score var, Score sd, Score count) :
      AbstractRegion(s, e, _id),
      _min(min),
      _max(max),
      _sum(sum),
      _median(median),
      _mean(mean),
      _var(var),
      _sd(sd),
      _count(count) {}

    virtual bool has_stats() const;
    Score min() const;
    Score max() const;
    Score sum() const;
    Score median() const;
    Score mean() const;
    Score var() const;
    Score sd() const;
    Score count() const;
    virtual size_t size() const;
    virtual RegionPtr clone() const;
  };

  RegionPtr build_simple_region(Position s, Position e, DatasetId _id);

  RegionPtr build_bed_region(Position s, Position e, DatasetId _id);

  RegionPtr build_stranded_region(Position s, Position e, DatasetId _id, std::string strand);

  RegionPtr build_wig_region(Position s, Position e, DatasetId _id, Score value);

  RegionPtr build_gene_region(Position s, Position e, DatasetId _id,
                              std::string source, Score score, std::string feature, std::string strand, std::string frame,
                              datatypes::Metadata& attributes, std::vector<datatypes::GeneOntologyTermPtr> go_terms);

  RegionPtr build_aggregte_region(Position s, Position e, DatasetId _id,
                                  Score min, Score max, Score sum, Score median, Score mean, Score var, Score sd, Score count);

  class Regions {
  private:
    std::vector<RegionPtr> _regions;

  public:
    Regions()
    {

    }

    Regions(size_t s)
    {
      _regions.reserve(s);
    }

    Regions(const Regions& other)
    {
      _regions.reserve(other._regions.size());
      for (const auto& r : other._regions) {
        _regions.emplace_back(r->clone());
      }
    }

    Regions(Regions&& r) noexcept :
      _regions(std::move(r._regions))
    { }


    Regions& operator=(const Regions& other)
    {
      if (&other == this) {
        return *this;
      }

      _regions = std::vector<RegionPtr>();
      _regions.reserve(other._regions.size());
      for (const auto& r : other._regions) {
        _regions.emplace_back(r->clone());
      }

      return *this;
    }


    template<typename RegionPtr>
    void emplace_back(RegionPtr && value)
    {
      _regions.emplace_back(std::forward<RegionPtr>(value));
    }

    RegionPtr& operator[] (const size_t pos)
    {
      return _regions[pos];
    }

    const RegionPtr& operator[] (const size_t pos) const
    {
      return _regions[pos];
    }

    bool empty() const
    {
      return _regions.empty();
    }

    size_t size() const
    {
      return _regions.size();
    }

    const RegionPtr& front() const
    {
      return _regions.front();
    }

    const RegionPtr& back() const
    {
      return _regions.back();
    }

    std::vector<RegionPtr>::iterator begin()
    {
      return _regions.begin();
    }

    std::vector<RegionPtr>::iterator end()
    {
      return _regions.end();
    }

    std::vector<RegionPtr>::const_iterator begin() const
    {
      return _regions.cbegin();
    }

    std::vector<RegionPtr>::const_iterator end() const
    {
      return _regions.cend();
    }

    void reserve(size_t new_cap)
    {
      _regions.reserve(new_cap);
    }

    void insert(std::vector<RegionPtr>::iterator it_end,
                std::move_iterator<std::vector<RegionPtr>::iterator> begin,
                std::move_iterator<std::vector<RegionPtr>::iterator> end)
    {
      _regions.insert(it_end, begin, end);
    }
  };

  typedef std::pair<std::string, Regions> ChromosomeRegions;
  typedef std::vector<ChromosomeRegions> ChromosomeRegionsList;


  // The value returned indicates whether the element passed as first argument is considered to go before the second in the specific strict weak ordering it defines.
  static struct {
    bool operator()(const RegionPtr &lhs, const RegionPtr &rhs) const
    {
      if (lhs->start() < rhs->start()) {
        return true;
      }

      if (rhs->start() < lhs->start()) {
        return false;
      }

      return lhs->end() < rhs->end();
    }
  } RegionPtrComparer;

} // namespace epidb

#endif