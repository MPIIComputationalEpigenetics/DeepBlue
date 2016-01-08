//
//  gtf_parser.hpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 07.09.15.
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