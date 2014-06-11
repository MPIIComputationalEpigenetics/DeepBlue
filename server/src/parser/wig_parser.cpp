//
//  wig_parser.cpp
//  epidb
//
//  Created by Felipe Albrecht on 17.01.14.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include <stdlib.h>
#include <cstdlib>
#include <cmath>
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>

#include "wig_parser.hpp"

#include "../extras/utils.hpp"

namespace epidb {
  namespace parser {

    WIGParser::WIGParser(const std::string& content) :
      actual_line_(0),
      input_(content),
      declare_track_(false),
      start(0),
      step(0),
      span(0)
    {}

    bool WIGParser::get_features(std::vector<Feature>& features, std::string& msg)
    {
      std::string line;

      while (!input_.eof()) {
        std::getline(input_, line);
        boost::trim(line);
        actual_line_++;
        if (!line.size()) {
          continue;
        }

        while (*line.rbegin() == '\\') {
          line = line.substr(0, line.size()-1);
          std::string new_line;
          std::getline(input_, new_line);
          boost::trim(new_line);
          actual_line_++;
          line = line + new_line;
        }

        if (line[0] == '#') {
          continue;
        }

        std::vector<std::string> strs;
        boost::split(strs, line, boost::is_any_of("\t "));

        if (strs[0].compare("browser") == 0) {
          continue;
        }

        std::map<std::string, std::string> info;
        if (strs[0].compare("track") == 0) {

          if (declare_track_)  {
            msg = "It is allowed only one track by file. Line: " + line_str();
            return false;
          }

          std::string s = line.substr(6); // remove "track "
          std::string separator1("");//dont let quoted arguments escape themselves
          std::string separator2(" \t");//split on spaces
          std::string separator3("\"\'");//let it have quoted arguments

          boost::escaped_list_separator<char> els(separator1,separator2,separator3);
          boost::tokenizer<boost::escaped_list_separator<char> > tok(s, els);

          for(boost::tokenizer<boost::escaped_list_separator<char> >::iterator beg=tok.begin(); beg!=tok.end();++beg) {
            std::vector<std::string> kv;
            size_t pos = beg->find_first_of("=");
            if (pos == std::string::npos) {
              msg = "The track seems to have an invalid <track> header line: " + *beg;
              return false;
            }
            std::string key = beg->substr(0,pos);
            std::string value = beg->substr(pos+1);
            info[key] = value;
          }

          declare_track_ = true;

          if (!last_feature_.empty()) {
            if (!check_feature(last_feature_, msg)) {
              return false;
            }
            features.push_back(last_feature_);
            last_feature_.clear();
          }

          continue;
        }

        if (strs[0].compare("variableStep") == 0 ||
          strs[0].compare("fixedStep") == 0) {

          params.clear();
          params["mode"] = strs[0];

          for (size_t i = 1; i < strs.size(); i++ ) {
            std::vector<std::string> kv;
            size_t pos = strs[i].find_first_of("=");
            if (pos == std::string::npos) {
              msg = "The track seems to have an invalid parameters: " + strs[i] + " . Line: " + line_str();
              return false;
            }

            std::string key = strs[i].substr(0,pos);
            std::string value = strs[i].substr(pos+1);
            boost::trim(value);

            if (value.size() == 0) {
              msg = "Value for the key " + key + " was not informed. Line: " + line_str();
              return false;
            }
            params[key] = value;
          }

          if (params.find("chrom") == params.end()) {
            msg = "The track doesn't specify a chromosome. Line: " + line_str();
            return false;
          }

          if (params.find("span") == params.end()) {
            span = 1;
          } else if (utils::string_to_long(params["span"], span)) {
            if (span == 0) {
              msg = "The span value should be bigger than 0. Line: " + line_str();
              return false;
            }
          } else {
            msg = "The track has a non integer or negative or null span value. Line: " + line_str();
            return false;
          }

          if (strs[0].compare("fixedStep") == 0) {
            if (params.find("start") == params.end() || !utils::is_number(params["start"])) {
              msg = "The track has a non integer or smaller than 1 as start value. Line: " + line_str();
              return false;
            }
            if (params.find("step") == params.end()) {
              msg = "The fixedStep tracks should define a positive step value. Line: " + line_str();
              return false;
            } else if (!utils::is_number(params["step"])) {
              msg = "The track has a negative or null step value. Line: " + line_str();
              return false;
            }
            start = atoi(params["start"].c_str());
            step = atoi(params["step"].c_str());
          }

          continue;
        }

        if (params.size() == 0) {
          msg = "The track is missing a fixedStep or variableStep directive. Line: "  + line_str();
        }

        Feature new_feature;

        if (params["mode"].compare("fixedStep") == 0) {

          if (span > step) {
            msg = "The span value (" + boost::lexical_cast<std::string>(span) + ") is bigger than the step value (" +
            boost::lexical_cast<std::string>(step) + "). Line: " + line_str();
            return false;
          }

          double value;
          if (!utils::string_to_double(line, value)) {
            msg = "The feature value " + line + " at the line " + line_str();
            return false;
          }
          new_feature.set(params["chrom"], start, start + span, value);
          start += step;

        } else if (params["mode"].compare("variableStep") == 0) {

          if (strs.size() != 2) {
            msg = "The track has invalid or missing values. Line: " + line_str();
            return false;
          }
          size_t position;
          double value;
          if (!utils::string_to_long(strs[0], position)) {
            msg = "The feature position " + strs[0] + " at the line " + line_str();
            return false;
          }
          if (!utils::string_to_double(strs[1], value)) {
            msg = "The feature value " + strs[0] + " at the line " + line_str();
            return false;
          }

          new_feature.set(params["chrom"], position, position+span, value);
        } else {
          msg = "The track is missing a fixedStep or variableStep directive at the line " + line_str();
          return false;
        }

        if (new_feature._value == 0.0) {
          continue;
        }

        if (!last_feature_.empty()) {
          if (!check_feature(last_feature_, msg)) {
            return false;
          }
          features.push_back(last_feature_);
          last_feature_.clear();
        }
        last_feature_ = new_feature;
      }

      if (!last_feature_.empty()) {
        if (!check_feature(last_feature_, msg)) {
          return false;
        }
        features.push_back(last_feature_);
        last_feature_.clear();
      }

      if (features.size() == 0) {
        msg = "The file does not inform any region or is empty.";
        return false;
      }

      return true;
    }
  }
}
