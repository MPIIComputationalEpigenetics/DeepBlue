//
//  get_regions.cpp
//  epidb
//
//  Created by Felipe Albrecht on 12.06.13.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include <limits>
#include <sstream>
#include <vector>
#include <map>

#include <boost/algorithm/string.hpp>

#include "../dba/dba.hpp"
#include "../dba/metafield.hpp"
#include "../dba/queries.hpp"
#include "../dba/key_mapper.hpp"

#include "../engine/commands.hpp"

#include "../extras/utils.hpp"
#include "../extras/serialize.hpp"

#include "../errors.hpp"
#include "../log.hpp"
#include "../regions.hpp"

namespace epidb {
  namespace command {

    class GetRegionsCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::OPERATIONS, "Gets the regions for the given query in the requested BED format.");
      }

      static Parameters parameters_()
      {
        Parameter p[] = {
          Parameter("query_id", serialize::STRING, "id of the query"),
          Parameter("output_format", serialize::STRING, "The columns that will be returned and theirs default value if necessary."),
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 3);
        return params;
      }

      static Parameters results_()
      {
        Parameter p[] = {
          Parameter("regions", serialize::STRING, "BED formated regions")
        };
        Parameters results(&p[0], &p[0] + 1);
        return results;
      }

    public:
      GetRegionsCommand() : Command("get_regions", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        const std::string query_id = parameters[0]->as_string();
        const std::string output_format = parameters[1]->as_string();
        const std::string user_key = parameters[2]->as_string();

        std::string msg;
        bool ok = false;
        if (!dba::check_user(user_key, ok, msg)) {
          result.add_error(msg);
          return false;
        }
        if (!ok) {
          std::string s = Error::m(ERR_INVALID_USER_KEY);
          result.add_error(s);
          return false;
        }

        if (!dba::check_query(user_key, query_id, ok, msg)) {
          result.add_error(msg);
          return false;
        }
        if (!ok) {
          result.add_error("Invalid query id.");
          return false;
        }

        ChromosomeRegionsList chromosomeRegionsList;
        if (!dba::query::retrieve_query(user_key, query_id, chromosomeRegionsList, msg)) {
          result.add_error(msg);
          return false;
        }

        std::map<int, parser::FileFormat> datasets_format;
        dba::Metafield metafield;
        std::stringstream ss;
        for (ChromosomeRegionsList::const_iterator it = chromosomeRegionsList.begin();
             it != chromosomeRegionsList.end(); it++) {
          std::string chromosome = it->first;

          Regions regions = it->second;

          if (regions->empty()) {
            continue;
          }

          if (it != chromosomeRegionsList.begin()) {
            ss << std::endl;
          }

          for (RegionsIterator cit = regions->begin() ; cit != regions->end(); cit++) {
            if (cit != regions->begin()) {
              ss << std::endl;
            }

            const Region &region = *cit;

            DatasetId id = region.dataset_id();

            std::vector<mongo::BSONObj> columns;
            if (!dba::query::get_columns_from_dataset(id, columns, msg)) {
              result.add_error(msg);
              return false;
            }

            parser::FileFormat format;
            if (!parser::FileFormatBuilder::build_for_outout(output_format, format, columns, msg)) {
              result.add_error(msg);
              return false;
            }
            if (!format_region(ss, chromosome, region, format, metafield, msg)) {
              result.add_error(msg);
              return false;
            }
          }
        }

        result.add_string(ss.str());

        return true;
      }

      inline bool format_region(std::stringstream &ss, const std::string &chromosome, const Region &region,
                                const parser::FileFormat &format, dba::Metafield &metafield, std::string &msg) const
      {
        std::string err;
        for (parser::FileFormat::const_iterator it =  format.begin(); it != format.end(); it++) {
          if (it != format.begin()) {
            ss << "\t";
          }
          if ( (*it)->name() == "CHROMOSOME") {
            ss << chromosome;
          } else if ((*it)->name() == "START") {
            ss << region.start();
          } else if ((*it)->name() == "END") {
            ss << region.end();
          } else if (dba::Metafield::is_meta((*it)->name())) {
            std::string result;
            if (!metafield.process((*it)->name(), chromosome, region, result, msg)) {
              return false;
            }
            if (result.empty()) {
              ss << (*it)->default_value();
            } else {
              ss << result;
            }
          } else {
            if ((*it)->type() == dba::columns::COLUMN_INTEGER) {
              Score v = region.value((*it)->internal_name());
              if (v == std::numeric_limits<Score>::min()) {
                ss << (*it)->default_value();
              } else {
                ss << utils::integer_to_string((int)v);
              }
            } else if ( ( (*it)->type() == dba::columns::COLUMN_DOUBLE) ||  ((*it)->type() == dba::columns::COLUMN_RANGE)) {
              Score v = region.value((*it)->internal_name());
              if (v == std::numeric_limits<Score>::min()) {
                ss << (*it)->default_value();
              } else {
                ss << utils::double_to_string(v);
              }
            } else {
              std::string o = region.get((*it)->internal_name());
              if (o.empty()) {
                ss << (*it)->default_value();
              } else {
                ss << o;
              }
            }
          }
        }
        return true;
      }

    } getRegionsCommand;
  }
}
