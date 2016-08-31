//
//  gene_expression_parser_factory.hpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 12.08.16.
//  Copyright (c) 2015 Max Planck Institute for Computer Science. All rights
//  reserved.
//
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

#ifndef GENE_EXPRESSION_PARSER_HPP
#define GENE_EXPRESSION_PARSER_HPP

#include <memory>

#include "../interfaces/serializable.hpp"

#include "../extras/utils.hpp"

namespace epidb {
  namespace parser {
    class IGeneExpressionParser {
    protected:
      size_t actual_line_;
      std::unique_ptr<std::istream> input_;

      const std::string line_str()
      {
        return utils::integer_to_string(actual_line_);
      }

    public:
      IGeneExpressionParser(std::unique_ptr<std::istream> &&input) :
        actual_line_(0),
        input_(std::move(input))
      {}

      virtual bool parse(ISerializableFilePtr& file, std::string &msg) = 0;

    };

    typedef std::unique_ptr<IGeneExpressionParser> GeneExpressionParserPtr;
  }
}


#endif