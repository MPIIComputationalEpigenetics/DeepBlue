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

#include <strtk.hpp>

#include "bedgraph_parser.hpp"
#include "wig.hpp"

#include "../types.hpp"

namespace epidb {
  namespace parser {

    struct bed_graph_line {
      std::string chr;
      Position start;
      Position end;
      double score;
    };

    BedGraphParser::BedGraphParser(const std::string &content) :
      WIGParser(content) {}

    bool BedGraphParser::get(parser::WigPtr &wig, std::string &msg)
    {
      wig = boost::shared_ptr<WigFile>(new WigFile());
      std::string current_chr;

      clock_t dsysTime = clock();
      clock_t init = clock();

      static const std::string delimiters = "\t ";
      strtk::for_each_line_conditional(input_, [&](const std::string & line) -> bool {

        actual_line_++;
        if (line.empty() || line[0] == '#' || line.compare(0, 7, "browser") == 0) {
          return true;
        }

        if (line.compare(0, 5, "track") == 0) {
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

        bed_graph_line row;
        if (!strtk::parse(line,"\t ",row.chr, row.start, row.end, row.score)) {
          msg = "Failed to parse line : " + utils::integer_to_string(actual_line_) + " " + line;
          return false;
        }

        if (!actual_track) {
          actual_track = build_bedgraph_track(row.chr);
        } else {
          if (actual_track->chromosome() != row.chr) {
            wig->add_track(actual_track);
            actual_track = build_bedgraph_track(row.chr);
          }
        }

        if (actual_line_ % 2500000 == 0) {
          std::cerr << (( ((float)  clock()) - dsysTime) / CLOCKS_PER_SEC) << std::endl;
          dsysTime = clock();
        }

        check_block_size(wig);
        actual_track->add_feature(row.start, row.end, row.score);

        return true;
      });

      if (actual_track) {
        wig->add_track(actual_track);
      }

      std::cerr << "Total: " << (( ((float)  clock()) - init) / CLOCKS_PER_SEC) << std::endl;
      return true;
    }
  }
}

