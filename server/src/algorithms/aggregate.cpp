//
//  aggregate.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 10.04.14.
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

#include <iostream>
#include <limits>
#include <set>
#include <vector>

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
      chr_regions = Regions();
      auto it_ranges = ranges.begin();

      DatasetId dataset_id = -1;
      dba::columns::ColumnTypePtr column;

      auto it_data_begin = data.begin();

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

        // Move to the begin of the range region
        while ( it_data_begin != data.end()
                && (*it_data_begin)->end() < (*it_ranges)->start() )  {
          it_data_begin++;
        }

        Accumulator acc;
        auto it_data = it_data_begin;
        while (it_data != data.end() &&
               (*it_ranges)->end() >= (*it_data)->start()) {

          if (((*it_data)->start() < (*it_ranges)->end()) && ((*it_data)->end() > (*it_ranges)->start())) {
            auto begin = std::max((*it_data)->start(), (*it_ranges)->start());
            auto end =  std::min((*it_data)->end(), (*it_ranges)->end());

            double overlap_length = end - begin;
            double original_length = (*it_data)->end() - (*it_data)->start();

            auto correct_offset = (overlap_length / original_length );

            if (field[0] == '@') {
              std::string value;
              if (!metafield.process(field, chrom, it_data->get(), status, value, msg)) {
                return false;
              }
              Score s;
              // TODO: the meta.process should return the double value directly
              utils::string_to_score(value, s);
              acc.push(s * correct_offset);
            } else if (field == "START") {
              acc.push((*it_data)->start());
            } else if (field == "END") {
              acc.push((*it_data)->end());
            } else {
              if (dataset_id != (*it_data)->dataset_id()) {
                dataset_id = (*it_data)->dataset_id();
                if (!dba::experiments::get_field_pos(dataset_id, field, column, msg)) {
                  return false;
                }
              }
              Score score = (*it_data)->value(column->pos());
              acc.push(score * correct_offset);
            }
          }
          it_data++;
        }

        chr_regions.emplace_back(build_aggregte_region((*it_ranges)->start(), (*it_ranges)->end(), DATASET_EMPTY_ID,
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
