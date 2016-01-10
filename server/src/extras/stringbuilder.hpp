//
//  stringbuilder.hpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 01.12.14.
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
