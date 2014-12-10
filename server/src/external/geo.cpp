//
//  add_sample_from_gsm.cpp
//  epidb
//
//  Created by Felipe Albrecht on 10.12.14.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include <sstream>
#include <string>

#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>

#include <strtk.hpp>

#import "../datatypes/metadata.hpp"

#import "url_loader.hpp"


namespace epidb {
  namespace external {
    namespace geo {

      const std::string GSM_QUERY_PAGE = "http://www.ncbi.nlm.nih.gov/geo/query/acc.cgi?targ=gsm&form=text&view=quick&acc=";

      bool parse_gsm(const std::string &content, datatypes::Metadata &metadata, std::string &msg)
      {
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
                return true;
              }
              if (key[0] == '!') {
                metadata[key] = value;
                return true;
              }
            }
          }
          return true;
        });

        return true;
      }

      bool load_gsm(const std::string &gsm_id, datatypes::Metadata &metadata, std::string &msg)
      {

        std::string url_address = GSM_QUERY_PAGE + gsm_id;

        std::string content;
        if (!url_loader::load(url_address, content, msg)) {
          return false;
        }

        if (!parse_gsm(content, metadata, msg)) {
          return false;
        }

        return true;
      }

    }
  }
}
