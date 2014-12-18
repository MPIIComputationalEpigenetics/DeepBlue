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
  StringBuilder::StringBuilder():
    tempstr(boost::filesystem::temp_directory_path().string() + boost::filesystem::unique_path().native())
  {
    ofs.open(tempstr.c_str(), std::ofstream::out);
  }

  StringBuilder::~StringBuilder()
  {
    if (ofs.is_open()) {
      ofs.close();
    }

    if (boost::filesystem::exists(tempstr.c_str())) {
      boost::filesystem::remove(tempstr.c_str());
    }
  }

  void StringBuilder::append(const std::string &src)
  {
    ofs << src;
  }

  void StringBuilder::append(std::string &&src)
  {
    ofs << src;
  }

  void StringBuilder::tab()
  {
    static std::string tab("\t");
    ofs << tab;
  }

  void StringBuilder::endLine()
  {
    static std::string new_line("\n");
    ofs << new_line;
  }

  std::string StringBuilder::to_string()
  {
    if (ofs.is_open()) {
      ofs.close();
    }

    std::ifstream file;
    file.open(tempstr.c_str());
    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    std::string buffer(size, ' ');
    file.seekg(0);
    file.read(&buffer[0], size);

    return buffer;
  }
}
