//
//  gtf_parser.hpp
//  epidb
//
//  Created by Felipe Albrecht on 07.09.15.
//  Copyright (c) 2016 Max Planck Institute for Informatics. All rights reserved.
//

#ifndef GTF_PARSER_HPP
#define GTF_PARSER_HPP

#include <memory>
#include <sstream>
#include <list>

#include <memory>

#include "gtf.hpp"

namespace epidb {
  namespace parser {

    class GTFParser {
    protected:
      size_t actual_line_;
      std::unique_ptr<std::istream> input_;
      bool parse_attributes(const std::string& line, const std::string& s_attributes, GTFRow::Attributes& attributes, std::string& msg);
      bool get_line(std::string line, std::string &msg);


      const std::string line_str()
      {
        return utils::integer_to_string(actual_line_);
      }

    public:
      GTFParser(std::unique_ptr<std::istream> &&input);
      bool get(GTFPtr &wig, std::string &msg);
      size_t actual_line();
      bool eof();
    };
  }
}

#endif /* defined(GTF_PARSER_HPP) */