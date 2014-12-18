//
//  stringbuilder.hpp
//  epidb
//
//  Created by Felipe Albrecht on 01.12.14.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

// Based on http://www.codeproject.com/Articles/647856/Performance-Improvement-with-the-StringBuilde

#ifndef EPIDB_STRINGBUILDER_HPP_
#define EPIDB_STRINGBUILDER_HPP_

#include <fstream>

namespace epidb {
  class StringBuilder {
    const std::string tempstr;
    std::ofstream ofs;

    StringBuilder(const StringBuilder &);
    StringBuilder &operator = (const StringBuilder &);

  public:
    StringBuilder();
    ~StringBuilder();

    void append(const std::string &src);
    void append(std::string &&src);
    void tab();
    void endLine();
    std::string to_string();
  };
}

#endif
