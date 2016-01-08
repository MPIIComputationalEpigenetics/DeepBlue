//
//  genome_data.h
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
//  Created by Felipe Albrecht on 07.07.13.
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

#ifndef EPIDB_PARSER_GENOME_DATA_HPP
#define EPIDB_PARSER_GENOME_DATA_HPP

#include <iostream>
#include <utility>
#include <vector>

#include <boost/algorithm/string.hpp>

namespace epidb {
  namespace parser {
    typedef std::pair<std::string, size_t> ChromosomeSize;
    typedef std::vector<ChromosomeSize> ChromosomesInfo;

    bool string_to_genome_info(const std::string& s, ChromosomesInfo& g, std::string& msg);
  }
}

#endif /* defined(EPIDB_PARSER_GENOME_DATA_HPP) */
