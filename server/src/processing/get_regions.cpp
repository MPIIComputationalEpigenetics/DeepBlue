//
//  get_regions.cpp
//  epidb
//
//  Created by Felipe Albrecht on 28.01.14.
//  Copyright (c) 2015 Max Planck Institute for Computer Science. All rights reserved.
//


#include <limits>
#include <vector>
#include <unordered_map>

#include "../datatypes/column_types_def.hpp"
#include "../datatypes/regions.hpp"

#include "../dba/dba.hpp"
#include "../dba/column_types.hpp"
#include "../dba/metafield.hpp"
#include "../dba/queries.hpp"

#include "../extras/utils.hpp"
#include "../extras/stringbuilder.hpp"

#include "../log.hpp"

namespace epidb {
  namespace processing {

    static bool process(const std::string &output_format, ChromosomeRegionsList &chromosomeRegionsList,
                        StringBuilder &sb, std::string &msg);

    static inline bool format_region(StringBuilder &sb, const std::string &chromosome, RegionPtr region,
                                     const parser::FileFormat &format, dba::Metafield &metafield, std::string &msg);

    bool get_regions(const std::string &query_id, const std::string &format, const std::string &user_key, StringBuilder &sb, std::string &msg)
    {

      ChromosomeRegionsList chromosomeRegionsList;
      if (!dba::query::retrieve_query(user_key, query_id, chromosomeRegionsList, msg)) {
        return false;
      }

      if (!process(format, chromosomeRegionsList, sb, msg)) {
        return false;
      }

      return true;
    }


    bool process(const std::string &output_format, ChromosomeRegionsList &chromosomeRegionsList,
                 StringBuilder &sb, std::string &msg)
    {
      std::unordered_map<DatasetId, parser::FileFormat> datasets_formats;
      dba::Metafield metafield;

      DatasetId actual_id = -1;
      parser::FileFormat format;

      for (ChromosomeRegionsList::iterator it = chromosomeRegionsList.begin();
           it != chromosomeRegionsList.end(); it++) {
        std::string chromosome = it->first;

        Regions &regions = it->second;

        if (regions.empty()) {
          continue;
        }

        if (it != chromosomeRegionsList.begin()) {
          sb.endLine();
        }

        for (auto cit = regions.begin() ; cit != regions.end(); cit++) {
          if (cit != regions.begin()) {
            sb.endLine();
          }

          RegionPtr region = std::move(*cit);
          DatasetId dataset_id = region->dataset_id();

          if (actual_id != dataset_id) {
            std::vector<mongo::BSONObj> columns;
            auto it = datasets_formats.find(dataset_id);
            if (it == datasets_formats.end()) {
              if (!dba::query::get_columns_from_dataset(dataset_id, columns, msg)) {
                return false;
              }
              parser::FileFormat new_format;
              if (!parser::FileFormatBuilder::build_for_outout(output_format, columns, new_format, msg)) {
                return false;
              }
              datasets_formats[dataset_id] = new_format;
              format = new_format;

            } else {
              auto format_it = datasets_formats.find(dataset_id);
              format = format_it->second;
            }
            actual_id = dataset_id;
          }

          if (!format_region(sb, chromosome, std::move(region), format, metafield, msg)) {
            return false;
          }
        }
      }
      return true;
    }

    static inline bool format_region(StringBuilder &sb, const std::string &chromosome, RegionPtr region,
                                     const parser::FileFormat &format, dba::Metafield &metafield, std::string &msg)
    {
      for (parser::FileFormat::const_iterator it =  format.begin(); it != format.end(); it++) {
        const dba::columns::ColumnTypePtr &column = *it;

        if (it != format.begin()) {
          sb.tab();
        }
        // Change to use column->pos()
        if ( column->name() == "CHROMOSOME") {
          sb.append(chromosome);

          // Change to use column->pos()
        } else if (column->name() == "START") {
          sb.append(utils::integer_to_string(region->start()));

          // Change to use column->pos()
        } else if (column->name() == "END") {
          sb.append(utils::integer_to_string(region->end()));
        } else if (dba::Metafield::is_meta(column->name())) {
          std::string result;
          if (!metafield.process(column->name(), chromosome, region.get(), result, msg)) {
            return false;
          }
          if (!result.empty()) {
            sb.append(std::move(result));
          }
        } else {
          if (column->type() == datatypes::COLUMN_CALCULATED) {
            std::string result;
            if (!column->execute(chromosome, region.get(), metafield, result, msg)) {
              return false;
            }
            sb.append(std::move(result));
          } else if (column->type() == datatypes::COLUMN_INTEGER) {
            const Score &v = region->value(column->pos());
            if (v != std::numeric_limits<Score>::min()) {
              sb.append(utils::integer_to_string((int)v));
            }
          } else if ( ( column->type() == datatypes::COLUMN_DOUBLE) ||  (column->type() == datatypes::COLUMN_RANGE)) {
            const Score &v = region->value(column->pos());
            if (v != std::numeric_limits<Score>::min()) {
              sb.append(utils::score_to_string(v));
            }
          } else {
            const std::string &o = region->get_string(column->pos());
            if (!o.empty()) {
              sb.append(o);
            }
          }
        }
      }
      return true;

    }

  }
}