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

#include "aggregate.hpp"
#include "accumulator.hpp"

#include "../datatypes/column_types_def.hpp"
#include "../datatypes/regions.hpp"

#include "../dba/experiments.hpp"
#include "../dba/key_mapper.hpp"
#include "../dba/metafield.hpp"

#include "../extras/utils.hpp"

#include "../errors.hpp"
#include "../log.hpp"

namespace epidb {
  namespace algorithms {

    bool aggregate_regions(const std::string &chrom, Regions &data, const Regions &ranges, const std::string &field,
                           dba::Metafield &metafield, processing::StatusPtr status, Regions &chr_regions, std::string &msg)
    {
      chr_regions = build_regions();
      auto it_ranges = ranges.begin();

      DatasetId dataset_id = -1;
      dba::columns::ColumnTypePtr column;

      while (it_ranges != ranges.end()) {

        // Check if processing was canceled
        bool is_canceled = false;
        if (!status->is_canceled(is_canceled, msg)) {
          return true;
        }
        if (is_canceled) {
          msg = Error::m(ERR_REQUEST_CANCELED);
          return false;
        }
        // ***

        Accumulator acc;
        auto it_data = data.begin();
        while (it_data != data.end()) {
          if (((*it_ranges)->start() <= (*it_data)->end()) && ((*it_ranges)->end() >= (*it_data)->end())) {
            if (field[0] == '@') {
              std::string value;
              if (!metafield.process(field, chrom, it_data->get(), status, value, msg)) {
                return false;
              }
              Score s;
              // TODO: the meta.process should return the double value directly
              utils::string_to_score(value, s);
              acc.push(s);
            } else {
              if (dataset_id != (*it_data)->dataset_id()) {
                dataset_id = (*it_data)->dataset_id();
                if (!dba::experiments::get_field_pos(dataset_id, field, column, msg)) {
                  return false;
                }
              }
              Score score = (*it_data)->value(column->pos());
              acc.push(score);
            }
          }
          it_data++;
        }

        chr_regions.push_back(build_aggregte_region((*it_ranges)->start(), (*it_ranges)->end(), DATASET_EMPTY_ID,
                              acc.min(), acc.max(), acc.median(), acc.mean(), acc.var(), acc.sd(), acc.count()));
        it_ranges++;
      }

      return true;
    }

    bool aggregate(ChromosomeRegionsList &data, ChromosomeRegionsList &ranges, const std::string &field,
                   processing::StatusPtr status, ChromosomeRegionsList &regions, std::string &msg)
    {
      // TODO :optimize it for finding the ChromosomeRegionsList data not in O(N) time
      dba::Metafield metafield;
      for (auto &range : ranges) {
        for (auto &datum : data) {
          Regions chr_regions;
          if (range.first == datum.first) {
            if (!aggregate_regions(range.first, datum.second, range.second, field, metafield, status, chr_regions, msg)) {
              return false;
            }
            std::pair<std::string, Regions> r(range.first, std::move(chr_regions));
            regions.push_back(std::move(r));
          }
        }
      }

      return true;
    }
  } // namespace algorithms
} // namespace epidb
