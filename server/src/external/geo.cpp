//
//  add_sample_from_gsm.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 10.12.14.
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

#include <sstream>
#include <string>

#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>

#include <strtk.hpp>

#include "../datatypes/metadata.hpp"

#include "url_loader.hpp"

#include "../errors.hpp"

namespace epidb {
  namespace external {
    namespace geo {

      const std::string GSM_QUERY_PAGE = "http://www.ncbi.nlm.nih.gov/geo/query/acc.cgi?targ=gsm&form=text&view=quick&acc=";

      bool parse_gsm(const std::string &content, datatypes::Metadata &metadata, std::string &msg)
      {
        bool found = false;
        std::stringstream ss(content);
        strtk::for_each_line_conditional(ss, [&](std::string & line) -> bool {
          boost::trim(line);
          if (line.empty())
          {
            return true;
          }

          if ((line[0] == '^') || (line[0] == '!'))
          {
            std::vector<std::string> strs;
            boost::split(strs, line, boost::is_any_of("="));

            if (strs.size() == 2) {
              std::string key = strs[0];
              std::string value = strs[1];

              boost::trim(key);
              boost::trim(value);

              if (key == "^SAMPLE") {
                metadata["GSM_SAMPLE"] = value;
                found = true;
                return true;
              }

              if (key == "!Sample_characteristics_ch1") {
                std::vector<std::string> sample_chars_strs;
                boost::split(sample_chars_strs, value, boost::is_any_of(":"));
                if (sample_chars_strs.size() == 2) {
                  key = "!"+sample_chars_strs[0];
                  value = sample_chars_strs[1];
                }
              }

              if (key[0] == '!') {
                key = key.substr(1);
                if (metadata.find(key) != metadata.end()) {
                  metadata[key] = metadata[key] + "\n" + value;
                } else {
                  metadata[key] = value;
                }
                return true;
              }
            }
          }
          return true;
        });

        return found;
      }

      bool load_gsm(const std::string &gsm_id, datatypes::Metadata &metadata, std::string &msg)
      {

        std::string url_address = GSM_QUERY_PAGE + gsm_id;

        std::string content;
        if (!url_loader::load(url_address, content, msg)) {
          return false;
        }

        if (!parse_gsm(content, metadata, msg)) {
          msg = Error::m(ERR_INVALID_GSM_IDENTIFIER, gsm_id);
          return false;
        }

        return true;
      }

    }
  }
}
