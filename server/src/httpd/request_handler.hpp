//
//  request_handler.hpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 27.05.13.
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
