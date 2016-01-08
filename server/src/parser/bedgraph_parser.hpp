//
//  bedgraph_parser.hpp
//  epidb
//
//  Created by Felipe Albrecht on 29.04.14.
//  Copyright (c) 2016 Max Planck Institute for Informatics. All rights reserved.
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