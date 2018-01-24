//
//  salmon_parser.hpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 24.01.17.
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

#ifndef SALMON_PARSER_HPP
#define SALMON_PARSER_HPP

#include <string>
#include <sstream>
#include <memory>

#include "../interfaces/serializable.hpp"

#include "gene_expression_parser.hpp"

namespace epidb {
  namespace parser {

    class SalmonParser: public IGeneExpressionParser {
    public:
      SalmonParser(std::unique_ptr<std::istream> &&input);
      virtual bool parse(ISerializableFilePtr &file, std::string &msg);
    };
  }
}

#endif /* defined(SALMON_PARSER_HPP) */