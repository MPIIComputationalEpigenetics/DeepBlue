//
//  aggregate.cpp
//  epidb
//
//  Created by Felipe Albrecht on 10.04.14.
//  Copyright (c) 2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include <cmath>
#include <iostream>
#include <limits>
#include <numeric>
#include <set>
#include <vector>

#include <boost/foreach.hpp>

#include "aggregate.hpp"

#include "../dba/key_mapper.hpp"
#include "../dba/metafield.hpp"

#include "../extras/utils.hpp"

#include "../regions.hpp"
#include "../log.hpp"

namespace epidb {
  namespace algorithms {

    class Accumulator {
    private:
      std::vector<double> values;
      bool _calculated;
      double _min;
      double _max;
      double _median;
      double _mean;
      double _var;
      double _sd;

    public:
      Accumulator() : _calculated(false),
        _min(0.0),
        _max(0.0),
        _median(0.0),
        _mean(0.0),
        _var(0.0),
        _sd(0.0) {}

      void push(double value)
      {
        values.push_back(value);
        _calculated = false;
      }

      double min()
      {
        calculate();
        return _min;
      }

      double max()
      {
        calculate();
        return _max;
      }

      double mean()
      {
        calculate();
        return _mean;
      }

      double var()
      {
        calculate();
        return _var;
      }

      double sd()
      {
        calculate();
        return _sd;
      }

      double median()
      {
        calculate();
        return _median;
      }

      double count()
      {
        return values.size();
      }

      void calculate()
      {
        if (_calculated || values.empty()) {
          return;
        }

        std::sort(values.begin(), values.end());

        _min = values[0];
        _max = values[values.size() - 1];
        _median = values[values.size() / 2];

        double sum = std::accumulate(values.begin(), values.end(), 0.0);
        _mean = sum / values.size();

        std::vector<double> diff(values.size());
        std::transform(values.begin(), values.end(), diff.begin(),
                       std::bind2nd(std::minus<double>(), _mean));
        double sq_sum = std::inner_product(diff.begin(), diff.end(), diff.begin(), 0.0);
        _var = sq_sum / values.size();
        _sd = std::sqrt(_var);

        _calculated = true;
      }
    };

    bool aggregate_regions(const std::string chrom, Regions &data, Regions &ranges, const std::string &field,
                                 dba::Metafield metafield, Regions &chr_regions, std::string& msg)
    {
      chr_regions = build_regions();
      RegionsConstIterator it_data = data->begin();
      RegionsConstIterator it_ranges = ranges->begin();

      Regions agg_regions;
      while (it_ranges != ranges->end()) {
        Accumulator acc;
        while (it_data != data->end() &&
               it_data->start() >= it_ranges->start() && it_data->start() <= it_ranges->end()) {

          if (it_data->start() >= it_ranges->start() && it_data->end() <= it_ranges->end()) {

            if (field[0] == '@') {
              std::string value;
              if (!metafield.process(field, chrom, *it_data, value, msg)) {
                std::cerr << msg << std::endl;
                return false;
              }
              double v;
              // TODO: the meta.process should return the double value directly
              utils::string_to_double(value, v);
              acc.push(v);
            } else {
              acc.push(it_data->value(field));
            }
          }
          it_data++;

        }
        Region region(it_ranges->start(), it_ranges->end(), DATASET_EMPTY_ID,
                      acc.min(), acc.max(), acc.median(), acc.mean(), acc.var(), acc.sd(), acc.count());

        chr_regions->push_back(region);
        it_ranges++;
      }

      return true;
    }

    bool aggregate(const ChromosomeRegionsList &data, const ChromosomeRegionsList &ranges, const std::string &field,
                         ChromosomeRegionsList &regions, std::string& msg)
    {
      //-- move to queries.cpp --//
      std::string field_value;
      if (field[0] != '@') {
        std::string short_field;
        std::string err;
        if (!dba::KeyMapper::to_short(field, short_field, err)) {
          EPIDB_LOG_ERR(err);
          return false;
        }
        field_value = short_field;
      } else {
        field_value = field;
      }
      //--- move to queries.cpp --//


      // TODO :optimize it for finding the ChromosomeRegionsList data in not O(N) time

      dba::Metafield metafield;
      BOOST_FOREACH(ChromosomeRegions range, ranges) {
        BOOST_FOREACH(ChromosomeRegions datum, data) {
          Regions chr_regions;
          if (range.first == datum.first) {
            if (!aggregate_regions(range.first, datum.second, range.second, field_value, metafield, chr_regions, msg)) {
              return false;
            }
            std::pair<std::string, Regions> r(range.first, chr_regions);
            regions.push_back(r);
          }
        }
      }

      return true;
    }
  } // namespace algorithms
} // namespace epidb
