//
//  header.hpp
//  epidb
//
//  Created by Felipe Albrecht on 27.05.13.
//  Copyright (c) 2016 Max Planck Institute for Informatics. All rights reserved.
//
//
// based on:
//
// header.hpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2012 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef EPIDB_HTTPD_HEADER_HPP
#define EPIDB_HTTPD_HEADER_HPP

#include <string>

namespace epidb {
  namespace httpd {

    struct header
    {
      std::string name;
      std::string value;
    };

  } // namespace httpd
} // namespace epidb

#endif // EPIDB_HTTPD_HEADER_HPP
