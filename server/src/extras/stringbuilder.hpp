//
//  stringbuilder.hpp
//  epidb
//
//  Created by Felipe Albrecht on 01.12.14.
//  Copyright (c) 2016 Max Planck Institute for Informatics. All rights reserved.
//

#ifndef EPIDB_STRINGBUILDER_HPP
#define EPIDB_STRINGBUILDER_HPP

#include <string>
#include <vector>

namespace epidb {
  class StringBuilder {
  private:
    using Buffer = std::vector<std::string>;

    Buffer buffer;
    size_t total_size;

    std::string block;
    static constexpr size_t MAX_BLOCK_SIZE = 4096;

  public:
    StringBuilder();
    StringBuilder(const StringBuilder &) = delete;
    StringBuilder & operator = (const StringBuilder &) = delete;

    void append(const std::string &src);
    void append(std::string &&src);
    void tab();
    void endLine();
    std::string to_string();

    bool empty();
  };
}

#endif
