//
//  fasta_parser.cpp
//  DeepBlue Epigenomic Data Server

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
//  Created by Felipe Albrecht on 31.03.13.
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