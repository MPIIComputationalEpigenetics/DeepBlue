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

#ifndef GENE_EXPRESSION_PARSER_FACTORY_HPP
#define GENE_EXPRESSION_PARSER_FACTORY_HPP

#include <memory>

#include "gene_expression_parser.hpp"

#include "cufflinks_parser.hpp"
#include "grape2_parser.hpp"
#include "salmon_parser.hpp"

namespace epidb {
  namespace parser {
    class GeneExpressionParserFactory {
    public:
      static GeneExpressionParserPtr build(const std::string& format, std::unique_ptr<std::istream> input, std::string &msg)
      {
        if (format == "cufflinks") {
          return std::unique_ptr<IGeneExpressionParser>(new CufflinksParser(std::move(input)));
        }

        if (format == "grape2") {
          return std::unique_ptr<IGeneExpressionParser>(new Grape2Parser(std::move(input)));
        }

        if (format == "salmon") {
          return std::unique_ptr<IGeneExpressionParser>(new SalmonParser(std::move(input)));
        }

        msg = "Unknow format " + format;

        return std::unique_ptr<IGeneExpressionParser>(nullptr);
      }
    };
  }
}


#endif