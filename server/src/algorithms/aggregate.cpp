//
//  aggregate.cpp
//  epidb
//
//  Created by Felipe Albrecht on 10.04.14.
//  Copyright (c) 2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include <iostream>
#include <limits>
#include <set>
#include <vector>

#include <boost/foreach.hpp>

#include "aggregate.hpp"
#include "accumulator.hpp"

#include "../datatypes/column_types_def.hpp"

#include "../dba/experiments.hpp"
#include "../dba/key_mapper.hpp"
#include "../dba/metafield.hpp"

#include "../extras/utils.hpp"

#include "../regions.hpp"
#include "../log.hpp"

namespace epidb {
  namespace algorithms {

    bool aggregate_regions(const std::string &chrom, const Regions &data, const Regions &ranges, const std::string &field,
                           dba::Metafield &metafield, Regions &chr_regions, std::string &msg)
    {
      chr_regions = build_regions();
      RegionsConstIterator it_data = data->begin();
      RegionsConstIterator it_ranges = ranges->begin();

      Regions agg_regions;

      int pos =1;
      DatasetId dataset_id = -1;
      datatypes::COLUMN_TYPES column_type;

      while (it_ranges != ranges->end()) {
        Accumulator acc;
        while (it_data != data->end() &&
               it_data->start() >= it_ranges->start() && it_data->start() <= it_ranges->end()) {

          if (it_data->start() >= it_ranges->start() && it_data->end() <= it_ranges->end()) {
            if (field[0] == '@') {
              std::string value;
              if (!metafield.process(field, chrom, *it_data, value, msg)) {
                return false;
              }
              double v;
              // TODO: the meta.process should return the double value directly
              utils::string_to_double(value, v);
              acc.push(v);
            } else {
              const Region &region = *it_data;
              if (dataset_id != region.dataset_id()) {
                dataset_id = region.dataset_id();
                if (!dba::experiments::get_field_pos(dataset_id, field, pos, column_type, msg)) {
                  return false;
                }
              }
              acc.push(it_data->value(pos));
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
                   ChromosomeRegionsList &regions, std::string &msg)
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


      // TODO :optimize it for finding the ChromosomeRegionsList data not in O(N) time

      dba::Metafield metafield;
      BOOST_FOREACH(const ChromosomeRegions & range, ranges) {
        BOOST_FOREACH(const ChromosomeRegions & datum, data) {
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
