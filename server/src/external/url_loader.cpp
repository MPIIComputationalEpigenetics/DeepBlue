//
//  url_loader.cpp
//  epidb
//
//  Created by Felipe Albrecht on 10.12.14.
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

#include <string>
#include <iostream>
#include <sstream>

#include <urdl/istream.hpp>

namespace epidb {
  namespace external {
    namespace url_loader {

      bool load(const std::string &url, std::string &content, std::string &msg)
      {
        // Open the URL. The connection is established and the HTTP request is sent.
        urdl::istream is(url);

        if (!is) {
          msg = "Unable to open URL " + url;
          return false;
        }

        std::stringstream ss;
        std::string line;
        while (std::getline(is, line)) {
          ss << line << std::endl;
        }

        content = ss.str();

        return true;
      }
    }
  }
}
