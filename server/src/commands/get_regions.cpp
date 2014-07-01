//
//  get_regions.cpp
//  epidb
//
//  Created by Felipe Albrecht on 12.06.13.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

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

#include "../log.hpp"
#include "../regions.hpp"

namespace epidb {
  namespace command {

    class GetRegionsCommand: public Command {

      typedef std::vector<std::pair<std::string, std::string> > Format;

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::OPERATIONS, "Gets the regions for the given query in the requested BED format.");
      }

      static Parameters parameters_()
      {
        Parameter p[] = {
          Parameter("query_id", serialize::STRING, "id of the query"),
          Parameter("user_format", serialize::STRING, "format of the returned regions"),
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
        const std::string user_format = parameters[1]->as_string();
        const std::string user_key = parameters[2]->as_string();

        std::string msg;
        bool ok = false;
        if (!dba::check_user(user_key, ok, msg)) {
          result.add_error(msg);
          return false;
        }
        if (!ok) {
          result.add_error("Invalid user key.");
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

        std::vector<std::string> format_names = utils::string_to_vector(user_format, ',');
        if (format_names.size() == 0) {
          result.add_error("Format must not be empty.");
          return false;
        }

        Format format;
        // load format columns and their defaults
        for (std::vector<std::string>::iterator it = format_names.begin(); it != format_names.end(); ++it) {
          std::vector<std::string> parts;
          boost::split(parts, *it, boost::is_any_of(":"));

          int size = parts.size();
          if (size == 0) {
            result.add_error("Format column must not be empty.");
            return false;
          } else if (size == 1) {
            boost::trim(parts[0]);
            if (parts[0] == "") {
              result.add_error("Format column must not be empty.");
              return false;
            }
            std::pair<std::string, std::string> column(parts[0], " ");
            format.push_back(column);
          } else if (size == 2) {
            boost::trim(parts[0]);
            if (parts[0] == "") {
              result.add_error("Format column must not be empty.");
              return false;
            }
            std::pair<std::string, std::string> column(parts[0], parts[1]);
            format.push_back(column);
          } else {
            result.add_error("Wrong output format: '" + *it + "'. The format is \"<field:default_value>,<field:default_value>,..\"");
            return false;
          }
        }

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
            if (!format_region(ss, chromosome, *cit, format, metafield, msg)) {
              result.add_error(msg);
              return false;
            }
          }
        }

        result.add_string(ss.str());

        return true;
      }

      bool format_region(std::stringstream &ss, const std::string &chromosome, const Region &region,
                         const Format &format, dba::Metafield &metafield, std::string &msg) const
      {
        std::string s;
        std::string err;
        for (Format::const_iterator it = format.begin(); it != format.end(); it++) {
          if (it != format.begin()) {
            ss << "\t";
          }

          if (it->first == "CHROMOSOME") {
            ss << chromosome;
          } else if (it->first == "START") {
            ss << region.start();
          } else if (it->first == "END") {
            ss << region.end();
          } else if (dba::Metafield::is_meta(it->first)) {
            std::string result;
            if (!metafield.process(it->first, chromosome, region, result, msg)) {
              return false;
            }
            if (result.empty()) {
              ss << it->second;
            } else {
              ss << result;
            }
          } else {
            if (!dba::KeyMapper::to_short(it->first, s, err)) {
              EPIDB_LOG_ERR(err);
              msg = err;
              return false;
            }
            std::string o = region.get(s);
            if (o.empty()) {
              ss << it->second;
            } else {
              ss << o;
            }
          }
        }
        return true;
      }

    } getRegionsCommand;
  }
}
