//
//  reply.hpp
//  epidb
//
//  Created by Felipe Albrecht on 27.05.13.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//
//
// based on:
//
// reply.hpp
// ~~~~~~~~~
//
// Copyright (c) 2003-2012 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef EPIDB_HTTPD_REPLY_HPP
#define EPIDB_HTTPD_REPLY_HPP

#include <string>
#include <vector>
#include <boost/asio.hpp>
#include "header.hpp"

namespace epidb {
  namespace httpd {


    struct Reply
    {
      enum ReplyType
      {
        ok = 200,
        created = 201,
        accepted = 202,
        no_content = 204,
        multiple_choices = 300,
        moved_permanently = 301,
        moved_temporarily = 302,
        not_modified = 304,
        bad_request = 400,
        unauthorized = 401,
        forbidden = 403,
        not_found = 404,
        request_limit_exceeded = 429,
        internal_server_error = 500,
        not_implemented = 501,
        bad_gateway = 502,
        service_unavailable = 503
      } type;

      std::vector<header> headers;
      std::string content;

      /// Convert the reply into a vector of buffers. The buffers do not own the
      /// underlying memory blocks, therefore the reply object must remain valid and
      /// not be changed until the write operation has completed.
      std::vector<boost::asio::const_buffer> to_buffers();

      /// Get a stock reply.
      static Reply stock_reply(Reply::ReplyType status, const std::string content);

      static Reply options_reply();
    };

  } // namespace epidb
} // namespace http

#endif // EPIDB_HTTPD_REPLY_HPP
