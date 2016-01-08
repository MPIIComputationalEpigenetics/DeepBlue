//
//  fasta_parser.hpp
//  epidb
//
//  Created by Felipe Albrecht on 31.03.13.
//  Copyright (c) 2016 Max Planck Institute for Informatics. All rights reserved.
//

#ifndef EPIDB_PARSER_FASTA_HPP
#define EPIDB_PARSER_FASTA_HPP

#include <string>

namespace epidb {
  namespace parser {
    namespace fasta {

      bool clean_up(const std::string& str, std::string& clean, std::string& msg );
    }
  }
}

#endif