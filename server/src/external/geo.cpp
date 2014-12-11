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

        std::cerr << content << std::endl;

        if (!parse_gsm(content, metadata, msg)) {
          msg = gsm_id + " is an invalid identifier.";
          return false;
        }

        return true;
      }

    }
  }
}
