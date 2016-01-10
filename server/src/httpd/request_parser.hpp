//
//  request_handler.cpp
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
// request_parser.hpp
// ~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2012 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef EPIDB_HTTPD_REQUEST_PARSER_HPP
#define EPIDB_HTTPD_REQUEST_PARSER_HPP

#include <iterator>
#include <boost/logic/tribool.hpp>
#include <boost/asio.hpp>

namespace epidb {
  namespace httpd {

    struct Request;

    /// Parser for incoming requests.
    class request_parser
    {
    public:
      /// Construct ready to parse the request method.
      request_parser();

      /// Reset to initial parser state.
      void reset();

      // true : ok
      // false : bad
      bool parse(Request& req, boost::asio::streambuf *buffer);

    private:
      /// Handle the next character of input.
      boost::tribool consume(Request& req, char input);

      /// Check if a byte is an HTTP character.
      static bool is_char(int c);

      /// Check if a byte is an HTTP control character.
      static bool is_ctl(int c);

      /// Check if a byte is defined as an HTTP tspecial character.
      static bool is_tspecial(int c);

      /// Check if a byte is a digit.
      static bool is_digit(int c);

      /// The current state of the parser.
      enum state
      {
        method_start,
        method,
        uri,
        http_version_h,
        http_version_t_1,
        http_version_t_2,
        http_version_p,
        http_version_slash,
        http_version_major_start,
        http_version_major,
        http_version_minor_start,
        http_version_minor,
        expecting_newline_1,
        header_line_start,
        header_lws,
        header_name,
        space_before_header_value,
        header_value,
        expecting_newline_2,
        expecting_newline_3
      } state_;
    };

  } // namespace httpd
} // namespace epidb

#endif // EPIDB_HTTPD_REQUEST_PARSER_HPP
