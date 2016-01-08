//
//  bedgraph_parser.cpp
//  epidb
//
//  Created by Felipe Albrecht on 29.04.14.
//  Copyright (c) 2016 Max Planck Institute for Informatics. All rights reserved.
//

#include <ctime>
#include <string>
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

    BedGraphParser::BedGraphParser(std::unique_ptr<std::istream> &&input) :
      WIGParser(std::move(input)) {}

    bool BedGraphParser::get(parser::WigPtr &wig, std::string &msg)
    {
      wig = std::shared_ptr<WigFile>(new WigFile());

      clock_t dsysTime = clock();
      clock_t init = clock();

      strtk::for_each_line_conditional(*input_, [&](const std::string & line) -> bool {

        actual_line_++;
        if (line.empty() || line[0] == '#' || line.compare(0, 7, "browser") == 0) {
          return true;
        }

        if (line.compare(0, 5, "track") == 0) {
          // Ignoring more than one TRACK
          /*
          if (declare_track_)  {
            msg = "It is allowed only one track by file. Line: " + line_str();
            return false;
          }
          */

          // Ignoring TRACK content
          /*
          std::map<std::string, std::string> info;
          if (!read_track(line, info, msg)) {
            return false;
          }
          */

          declare_track_ = true;
          return true;
        }

        bed_graph_line row;
        if (!strtk::parse(line,"\t ",row.chr, row.start, row.end, row.score)) {
          msg = "Failed to parse line : " + line_str() + " " + line;
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

      // Verify error
      if (!msg.empty()) {
        return false;
      }

      if (actual_track) {
        wig->add_track(actual_track);
      }

      std::cerr << "Total: " << (( ((float)  clock()) - init) / CLOCKS_PER_SEC) << std::endl;
      return true;
    }
  }
}

