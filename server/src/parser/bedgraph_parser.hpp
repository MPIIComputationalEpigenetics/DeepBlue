//
//  bedgraph_parser.hpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 29.04.14.
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

#ifndef BEDGRAPH_PARSER_HPP
#define BEDGRAPH_PARSER_HPP

#include <sstream>
#include <list>

#include <memory>

#include "wig_parser.hpp"
#include "wig.hpp"

namespace epidb {
  namespace parser {

    class BedGraphParser : public WIGParser {
    public:
    	BedGraphParser(std::unique_ptr<std::istream> &&input);
    	bool get(parser::WigPtr& wig, std::string &msg);
    };
  }
}



#endif /* BEDGRAPH_PARSER_HPP */