//
//  stringbuilder.cpp
//  epidb
//
//  Created by Felipe Albrecht on 01.12.14.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include "stringbuilder.hpp"

#include <sstream>
#include <fstream>
#include <iostream>

#include <boost/filesystem.hpp>

namespace epidb {
  StringBuilder::StringBuilder() :
    total_size(0)
  { }

  void StringBuilder::append(const std::string &src)
  {
    buffer.emplace_back(src);
    total_size += src.size();
  }

  void StringBuilder::append(std::string &&src)
  {
    buffer.emplace_back(std::move(src));
    total_size += src.size();
  }

  void StringBuilder::tab()
  {
    static std::string tab("\t");
    buffer.emplace_back(tab);
    total_size++;
  }

  void StringBuilder::endLine()
  {
    static std::string new_line("\n");
    buffer.emplace_back(new_line);
    total_size++;
  }

  std::string StringBuilder::to_string()
  {
    std::string result;
    result.reserve(total_size + 1);

    for (auto iter = buffer.begin(); iter != buffer.end(); ++iter) {
      result += *iter;
      std::string().swap(*iter);
    }

    return result;
  }
}
