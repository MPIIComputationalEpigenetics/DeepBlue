//
//  url_loader.cpp
//  epidb
//
//  Created by Felipe Albrecht on 10.12.14.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include <string>
#include <iostream>
#include <ostream>

#include <urdl/istream.hpp>

#include "../extras/stringbuilder.hpp"

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

        StringBuilder<char> sb;
        std::string line;
        while (std::getline(is, line)) {
          sb.append(std::move(line));
          sb.endLine();
        }

        content = sb.ToString();

        return true;
      }
    }
  }
}
