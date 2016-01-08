//
//  request_handler.hpp
//  epidb
//
//  Created by Felipe Albrecht on 27.05.13.
//  Copyright (c) 2016 Max Planck Institute for Informatics. All rights reserved.
//
//
// based on:
//
// request_handler.hpp
// ~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2012 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef EPIDB_HTTPD_REQUEST_HANDLER_HPP
#define EPIDB_HTTPD_REQUEST_HANDLER_HPP

#include <string>
#include <boost/noncopyable.hpp>

// #include "rate_limiter.hpp"
#include "reply.hpp"

namespace epidb {
  namespace httpd {

    struct Request;

    /// The common handler for all incoming requests.
    class request_handler
    : private boost::noncopyable
    {
     public:
      /// Handle a request and produce a reply.
      Reply handle_request(const Request& req, std::string& content);

     private:
      const std::string process_content(const Request& request, std::string& content, bool& ok);
    };

  } // namespace httpd
} // namespace epidb

#endif // EPIDB_HTTPD_REQUEST_HANDLER_HPP
