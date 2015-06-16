//
//  request.hpp
//  epidb
//
//  Created by Felipe Albrecht on 27.05.13.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//
//
// based on:
//
// request.hpp
// ~~~~~~~~~~~
//
// Copyright (c) 2003-2012 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef EPIDB_HTTPD_REQUEST_HPP
#define EPIDB_HTTPD_REQUEST_HPP

#include <string>
#include <vector>
#include "header.hpp"

namespace epidb {
  namespace httpd {

    /// A request received from a client.
    struct Request {
      unsigned long long id_;
      std::string method;
      std::string path;
      std::string ip;
      int http_version_major;
      int http_version_minor;
      std::vector<header> headers;
      size_t content_length;
      Request(unsigned long long id) :
        id_(id),
        http_version_major(0),
        http_version_minor(0),
        content_length(0) {}
    };
  }
}

#endif