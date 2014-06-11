//
//  fasta_parser.cpp
//  epidb
//
//  Created by Felipe Albrecht on 31.03.13.
//  Copyright (c) 2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include <algorithm>
#include <string>
#include <sstream>

#include <boost/algorithm/string.hpp>

namespace epidb {
  namespace parser {
    namespace fasta {

      bool clean_up(const std::string& str, std::string& clean, std::string& msg )
      {
        clean.reserve(str.length());

        std::stringstream input(str);
        if (input.eof()) {
          msg = "Empty content.";
          return false;
        }

        std::string line;
        std::getline(input, line);
        boost::trim(line);
        if (line[0] != '>') {
          clean.append(line);
        }

        while (!input.eof()) {
          std::getline(input, line);
          boost::trim(line);
          if (line[0] != '>') {
            clean.append(line);
          } else {
            msg = "It is only accepted one sequence per FASTA file.";
            return false;
          }
        }

        return true;
      }
    }
  }
}