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

#include "../types.hpp"

namespace epidb {
  namespace parser {


    BedGraphParser::BedGraphParser(const std::string &content) :
      WIGParser(content) {}

    bool BedGraphParser::get(parser::WigPtr &wig, std::string &msg)
    {
      wig = boost::shared_ptr<WigFile>(new WigFile());
      std::string line;
      std::string current_chr;

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

        Position start;
        Position end;
        Score score;

        if (!utils::string_to_position(strs[1], start)) {
          msg = "The start position '" + line + "' is not valid. (Line: " + line_str() + ")";
          return false;
        }

        if (!utils::string_to_position(strs[2], end)) {
          msg = "The end position '" + line + "'' is not valid. (Line: " + line_str() + ")";
          return false;
        }

        if (!utils::string_to_score(strs[3], score)) {
          msg = "The score '" + line + "' is not valid. (Line: " + line_str() + ")";
          return false;
        }

        if (actual_line_ % 600000 == 0){
          std::cerr << (( ((float)  clock()) - dsysTime)/CLOCKS_PER_SEC) << std::endl;
          dsysTime = clock();
        }

        check_block_size(wig);
        actual_track->add_feature(start, end, score);
      }
      if (actual_track) {
        wig->add_track(actual_track);
      }
      return true;
    }
  }
}

