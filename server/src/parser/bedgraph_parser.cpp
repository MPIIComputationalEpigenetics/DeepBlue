//
//  bedgraph_parser.cpp
//  epidb
//
//  Created by Felipe Albrecht on 29.04.14.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include <string>


#include <boost/algorithm/string.hpp>

#include <ctime>

#include "bedgraph_parser.hpp"
#include "wig.hpp"

namespace epidb {
  namespace parser {


    BedGraphParser::BedGraphParser(const std::string &content) :
      WIGParser(content) {}

    bool BedGraphParser::get(parser::WigPtr &wig, std::string &msg)
    {
      wig = boost::shared_ptr<WigFile>(new WigFile());
      std::string line;
      std::string current_chr;

      size_t prev_start;
      size_t prev_end;

      clock_t dsysTime = clock();

      while (!input_.eof()) {
        std::getline(input_, line);
        boost::trim(line);
        actual_line_++;
        if (line.empty()) {
          continue;
        }

        std::vector<std::string> strs;
        boost::split(strs, line, boost::is_any_of("\t "));

        if (strs[0] == "browser") {
          continue;
        }

        if (strs[0] == "track") {
          if (declare_track_)  {
            msg = "It is allowed only one track by file. Line: " + line_str();
            return false;
          }

          std::map<std::string, std::string> info;
          if (!read_track(line, info, msg)) {
            return false;
          }

          declare_track_ = true;
          continue;
        }

        // Code for ENCODE bigbed files
        if ((strs[0] == "#bedGraph") && (strs[1] == "section") && strs.size() == 3) {
          std::vector<std::string> location;
          boost::split(location, strs[2], boost::is_any_of(":"));
          if (location.size() == 2) {
            std::string chromosome = location[0];
            std::vector<std::string> start_end;
            boost::split(start_end, location[1], boost::is_any_of("-"));
            if (start_end.size() == 2) {
              size_t start;
              size_t end;
              if (!utils::string_to_long(start_end[0], start)) {
                msg = "The start position " + line + " is not valid. (Line: " + line_str() + ")";
                return false;
              }

              if (!utils::string_to_long(start_end[1], end)) {
                msg = "The end position " + line + " is not valid. (Line: " + line_str() + ")";
                return false;
              }

              if (actual_track) {
                wig->add_track(actual_track);
              }
              actual_track = build_bedgraph_track(chromosome, start, end);
            }
          }
          continue;
        }

        if (line[0] == '#') {
          continue;
        }

        if (strs.size() != 4) {
          msg = "Invalid line (" + line_str() + ") : " + line;
          return false;
        }

        std::string chromosome = strs[0];

        if (!actual_track) {
          actual_track = build_bedgraph_track(chromosome);
        } else {
          if (actual_track->chromosome() != chromosome) {
            wig->add_track(actual_track);
            actual_track = build_bedgraph_track(chromosome);
          }
        }

        size_t start;
        size_t end;
        float score;

        if (!utils::string_to_long(strs[1], start)) {
          msg = "The start position '" + line + "' is not valid. (Line: " + line_str() + ")";
          return false;
        }

        if (!utils::string_to_long(strs[2], end)) {
          msg = "The end position '" + line + "'' is not valid. (Line: " + line_str() + ")";
          return false;
        }

        if (!utils::string_to_float(strs[3], score)) {
          msg = "The score '" + line + "' is not valid. (Line: " + line_str() + ")";
          return false;
        }


        if (actual_line_ % 300000 == 0){
          std::cerr << (( ((float)  clock()) - dsysTime)/CLOCKS_PER_SEC) << std::endl;
          dsysTime = clock();
        }

        actual_track->add_feature(start, end, score);

        prev_start = start;
        prev_end = end;
      }
      if (actual_track) {
        wig->add_track(actual_track);
      }
      return true;
    }
  }
}

