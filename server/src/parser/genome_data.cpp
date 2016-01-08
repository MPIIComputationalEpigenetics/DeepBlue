//
//  genome_data.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 07.07.13.
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

#include <iostream>
#include <sstream>
#include <string>

#include <boost/algorithm/string.hpp>

#include "genome_data.hpp"

#include "../extras/utils.hpp"

namespace epidb {
  namespace parser {
    // TODO: check exceptions
    bool string_to_genome_info(const std::string& s, ChromosomesInfo& g, std::string& msg)
    {
      std::stringstream ss(s);

      while (!ss.eof()) {
        std::string line;
        std::getline(ss, line);
        boost::trim(line);

        if (!line.size())
          continue;

        std::stringstream ss_line(line);

        std::string name;
        ss_line >> name;

        if (ss_line.eof()) {
          msg =  "The size of the chromosome " + name + " is missing.";
          return false;
        }

        std::string s_size;
        ss_line >> s_size;

        size_t size;
        if (!utils::string_to_long(s_size, size)) {
          msg = "Invalid size " + s_size;
          return false;
        }
        ChromosomeSize cs(name, size);
        g.push_back(cs);
      }
      return true;
    }
  }
}
