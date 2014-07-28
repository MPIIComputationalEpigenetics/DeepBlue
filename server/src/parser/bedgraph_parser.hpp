//
//  bedgraph_parser.hpp
//  epidb
//
//  Created by Felipe Albrecht on 29.04.14.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#ifndef BEDGRAPH_PARSER_HPP
#define BEDGRAPH_PARSER_HPP

#include <sstream>
#include <list>

#include <boost/shared_ptr.hpp>

#include "wig_parser.hpp"
#include "wig.hpp"

namespace epidb {
  namespace parser {

    class BedGraphParser : public WIGParser {
    public:
    	BedGraphParser(const std::string &content);
    	bool get(parser::WigPtr& wig, std::string &msg);
    };
  }
}



#endif /* BEDGRAPH_PARSER_HPP */