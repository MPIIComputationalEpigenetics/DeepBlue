//
//  processing.hpp
//  epidb
//
//  Created by Felipe Albrecht on 28.01.15.
//  Copyright (c) 2015 Max Planck Institute for Computer Science. All rights reserved.
//

#ifndef EPIDB_PROCESSING_PROCESSING_HPP
#define EPIDB_PROCESSING_PROCESSING_HPP

#include <string>

namespace epidb {

  class StringBuilder;

  namespace processing {
    bool count_regions(const std::string &query_id, const std::string &user_key, size_t &count, std::string &msg);

    bool get_regions(const std::string &query_id, const std::string &format, const std::string &user_key, StringBuilder &sb, std::string &msg);
  }
}

#endif
