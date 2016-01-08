//
//  genome_data.h
//  epidb
//
//  Created by Felipe Albrecht on 07.07.13.
//  Copyright (c) 2016 Max Planck Institute for Informatics. All rights reserved.
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
