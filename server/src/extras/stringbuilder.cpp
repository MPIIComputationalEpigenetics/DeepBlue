//
//  stringbuilder.cpp
//  DeepBlue Epigenomic Data Server

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
//  Created by Felipe Albrecht on 01.12.14.
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

#include "stringbuilder.hpp"

#include <sstream>
#include <fstream>
#include <iostream>

#include <boost/filesystem.hpp>

namespace epidb {
  StringBuilder::StringBuilder() :
    total_size(0)
  {
    block.reserve(MAX_BLOCK_SIZE);
  }

  void StringBuilder::append(const std::string &src)
  {
    if (block.size() + src.size() > MAX_BLOCK_SIZE) {
      if (!block.empty()) {
        buffer.emplace_back(std::move(block));
        block.clear();
        block.reserve(MAX_BLOCK_SIZE);
      }
    }

    block += src;
    total_size += src.size();
  }

  void StringBuilder::append(std::string &&src)
  {
    if (block.size() + src.size() > MAX_BLOCK_SIZE) {
      if (!block.empty()) {
        buffer.emplace_back(std::move(block));
        block.clear();
        block.reserve(MAX_BLOCK_SIZE);
      }
    }

    block += src;
    total_size += src.size();
  }

  void StringBuilder::tab()
  {
    static std::string tab("\t");
    append(tab);
  }

  void StringBuilder::endLine()
  {
    static std::string new_line("\n");
    append(new_line);
  }

  std::string StringBuilder::to_string()
  {
    buffer.emplace_back(block);

    std::string result;
    result.reserve(total_size + 1);

    for (auto iter = buffer.begin(); iter != buffer.end(); ++iter) {
      result += *iter;
      std::string().swap(*iter);
    }

    return result;
  }

  bool StringBuilder::empty()
  {
    return total_size == 0;
  }
}
