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
#include <ctime>
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <memory>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>
#include <boost/shared_ptr.hpp>

#include <strtk.hpp>

#include "wig_parser.hpp"

#include "../extras/utils.hpp"
#include "wig.hpp"

namespace epidb {
  namespace parser {

    WIGParser::WIGParser(std::unique_ptr<std::istream> &&input) :
      actual_line_(0),
      input_(std::move(input)),
      declare_track_(false),
      actual_track(),
      map_overlap_counter()
    {}

    bool WIGParser::read_track(const std::string &line, std::map<std::string, std::string> &info, std::string &msg)
    {
      std::string s = line.substr(6); // remove "track "
      std::string separator1("");//dont let quoted arguments escape themselves
      std::string separator2(" \t");//split on spaces
      std::string separator3("\"\'");//let it have quoted arguments

      boost::escaped_list_separator<char> els(separator1, separator2, separator3);
      boost::tokenizer<boost::escaped_list_separator<char> > tok(s, els);

      for (boost::tokenizer<boost::escaped_list_separator<char> >::iterator beg = tok.begin(); beg != tok.end(); ++beg) {
        std::vector<std::string> kv;
        size_t pos = beg->find_first_of("=");
        if (pos == std::string::npos) {
          msg = "The track seems to have an invalid <track> header line: " + *beg;
          return false;
        }
        std::string key = beg->substr(0, pos);
        std::string value = beg->substr(pos + 1);
        info[key] = value;
      }
      return true;
    }

    bool WIGParser::read_parameters(const std::vector<std::string> &strs, std::map<std::string, std::string> &params,
                                    std::string &msg)
    {
      params.clear();

      for (size_t i = 1; i < strs.size(); i++ ) {
        std::vector<std::string> kv;
        size_t pos = strs[i].find_first_of("=");
        if (pos == std::string::npos) {
          msg = "The track seems to have an invalid parameters: " + strs[i] + " . Line: " + line_str();
          return false;
        }

        std::string key = strs[i].substr(0, pos);
        std::string value = strs[i].substr(pos + 1);
        boost::trim(value);

        if (value.size() == 0) {
          msg = "Value for the key " + key + " was not informed. Line: " + line_str();
          return false;
        }
        params[key] = value;
      }
      return true;
    }

    bool WIGParser::read_format(std::string &line, TrackPtr &track, std::string &msg)
    {
      boost::trim(line);
      std::vector<std::string> strs;
      boost::split(strs, line, boost::is_any_of("\t "));

      std::map<std::string, std::string> params;
      if (!read_parameters(strs, params, msg)) {
        return false;
      }

      std::string mode(strs[0]);
      int start;
      int step;
      int span;

      if (params.find("chrom") == params.end()) {
        msg = "The track doesn't specify a chromosome. Line: " + line_str();
        return false;
      }
      std::string chrom = params["chrom"];

      if (params.find("span") == params.end()) {
        span = 1;
      } else if (utils::string_to_int(params["span"], span)) {
        if (span <= 0) {
          msg = "The span value should be bigger than 0. Line: " + line_str();
          return false;
        }
      } else {
        msg = "The track has a non integer or negative or null span value. Line: " + line_str();
        return false;
      }

      if (mode == "fixedStep") {
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
        if (!utils::string_to_int(params["start"], start)) {
          msg = "The start value is not valid. Line: " + line_str();
        }

        if (!utils::string_to_int(params["step"], step)) {
          msg = "The step value is not valid. Line: " + line_str();
        }

        if (step <= 0) {
          msg = "The step value should be bigger than 0. Line: " + line_str();
        }

        if (start < 0) {
          msg = "The start value should be positive. Line: " + line_str();
        }

        if (span > step) {
          msg = "The span value (" + utils::integer_to_string(span) + ") is bigger than the step value (" +
                utils::integer_to_string(step) + "). Line: " + line_str();
          return false;
        }
        track = build_fixed_track(chrom, start, step, span);
        return true;

      } else if (mode == "variableStep") {
        track = build_variable_track(chrom, span);
        return true;

      } else {
        msg = "Invalid track mode " + mode + ". Line: " + line_str();
        return false;
      }
    }

    bool WIGParser::check_feature(const std::string &chromosome, const size_t start, const size_t span, const size_t &line, std::string &msg)
    {
      boost::shared_ptr<boost::icl::interval_set<int> > overlap_counter;

      if (map_overlap_counter.find(chromosome) == map_overlap_counter.end()) {
        overlap_counter = boost::shared_ptr<boost::icl::interval_set<int> >( new boost::icl::interval_set<int>());
        map_overlap_counter[chromosome] = overlap_counter;
      } else {
        overlap_counter = map_overlap_counter[chromosome];
      }

      boost::icl::discrete_interval<int> inter_val =  boost::icl::discrete_interval<int>::right_open(start, start + span);

      if (boost::icl::intersects(*overlap_counter, inter_val)) {
        msg = "The region " + chromosome + " " + utils::integer_to_string(start) + " " +
              utils::integer_to_string(start + span) + " is overlaping with some other region. Line: " +
              utils::integer_to_string(line);
        return false;
      }
      (*overlap_counter) += inter_val;
      return true;
    }

    void WIGParser::check_block_size(WigPtr &wig)
    {
      if (actual_track->features() >= BLOCK_SIZE) {
        wig->add_track(actual_track);
        actual_track = actual_track->split();
      }
    }

    bool WIGParser::get(WigPtr &wig, std::string &msg)
    {
      clock_t init = clock();

      wig = boost::shared_ptr<WigFile>(new WigFile());
      strtk::for_each_line_conditional(*input_, [&](std::string & line) -> bool {
        actual_line_++;
        if (line.empty())
        {
          return true;
        }

        while (*line.rbegin() == '\\')
        {
          line = line.substr(0, line.size() - 1);
          std::string new_line;
          std::getline(*input_, new_line);
          boost::trim(new_line);
          actual_line_++;
          line = line + new_line;
        }

        if (line.empty() || line[0] == '#' || line.compare(0, 7, "browser") == 0)
        {
          return true;
        }

        if (line.compare(0, 5, "track") == 0)
        {
          if (declare_track_)  {
            msg = "It is allowed only one track by file. Line: " + line_str();
            return false;
          }

          std::map<std::string, std::string> info;
          if (!read_track(line, info, msg)) {
            return false;
          }

          declare_track_ = true;
          return true;
        }

        if ((line.compare(0, 12, "variableStep") == 0)  ||
            line.compare(0, 9, "fixedStep") == 0)
        {

          TrackPtr new_track;
          if (!read_format(line, new_track, msg)) {
            return false;
          }

          if (!actual_track) {
            actual_track = new_track;
          } else if ((actual_track->chromosome() != new_track->chromosome())  ||
                     (actual_track->end() != new_track->start())) {
            if (actual_track) {
              wig->add_track(actual_track);
            }
            actual_track = new_track;
          }

          return true;
        }

        if (!actual_track)
        {
          msg = "The track is missing a fixedStep or variableStep directive. Line: "  + line_str();
          return false;
        }

        if (actual_track->type() == FIXED_STEP)
        {
          double value;
          if (!utils::string_to_double(line, value)) {
            msg = "The feature value " + line + " at the line " + line_str() + " is not a valid number";
            return false;
          }

          if (!check_feature(actual_track->chromosome(),
                             actual_track->start() + ( actual_track->features() * actual_track->span()),
                             actual_track->span(), actual_line_, msg)) {
            return false;
          }

          check_block_size(wig);

          actual_track->add_feature(value);

        } else if (actual_track->type() == VARIABLE_STEP)
        {
          size_t position;
          double value;

          if (!strtk::parse(line, "\t ", position, value)) {
            msg = "Failed to parse line : " + line_str() + " " + line;
            return false;
          }

          if (!check_feature(actual_track->chromosome(), position, actual_track->span(), actual_line_, msg)) {
            return false;
          }

          actual_track->add_feature(position, value);
        } else {
          msg = "The track is missing a fixedStep or variableStep directive at the line " + line_str();
          return false;
        }
        return true;
      });

      // Verify error
      if (!msg.empty()) {
        return false;
      }

      if (actual_track) {
        wig->add_track(actual_track);
      }

      if (wig->size() == 0) {
        msg = "The file does not inform any region or is empty.";
        return false;
      }

      std::cerr << "Total: " << (( ((float)  clock()) - init) / CLOCKS_PER_SEC) << std::endl;
      return true;
    }
  }
}
