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
// request_parser.cpp
// ~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2012 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "request_parser.hpp"
#include "request.hpp"

namespace epidb {
  namespace httpd {

    request_parser::request_parser()
    : state_(method_start)
    {
    }

    void request_parser::reset()
    {
      state_ = method_start;
    }

    bool request_parser::parse(Request& req, boost::asio::streambuf *buffer)
    {
      std::istreambuf_iterator<char> eos;
      std::istreambuf_iterator<char> iit (buffer);

      boost::tribool result;
      std::size_t count(0);

      result = boost::indeterminate;
      while (iit != eos && indeterminate(result))
      {
        result = consume(req, *iit++);
        count++;
      }

      if (!result)
        return false;

      for(std::vector<header>::const_iterator it = req.headers.begin(); it != req.headers.end(); it++)
      {
        if(it->name == "Content-Length") {
          std::istringstream s(it->value);
          s >> req.content_length;
          break;
        }
      }

      return true;
    }

    boost::tribool request_parser::consume(Request& req, char input)
    {
      switch (state_)
      {
        case method_start:
          if (!is_char(input) || is_ctl(input) || is_tspecial(input))
          {
            return false;
          }
          else
          {
            state_ = method;
            req.method.push_back(input);
            return boost::indeterminate;
          }
        case method:
          if (input == ' ')
          {
            state_ = uri;
            return boost::indeterminate;
          }
          else if (!is_char(input) || is_ctl(input) || is_tspecial(input))
          {
            return false;
          }
          else
          {
            req.method.push_back(input);
            return boost::indeterminate;
          }
        case uri:
          if (input == ' ')
          {
            state_ = http_version_h;
            return boost::indeterminate;
          }
          else if (is_ctl(input))
          {
            return false;
          }
          else
          {
            req.path.push_back(input);
            return boost::indeterminate;
          }
        case http_version_h:
          if (input == 'H')
          {
            state_ = http_version_t_1;
            return boost::indeterminate;
          }
          else
          {
            return false;
          }
        case http_version_t_1:
          if (input == 'T')
          {
            state_ = http_version_t_2;
            return boost::indeterminate;
          }
          else
          {
            return false;
          }
        case http_version_t_2:
          if (input == 'T')
          {
            state_ = http_version_p;
            return boost::indeterminate;
          }
          else
          {
            return false;
          }
        case http_version_p:
          if (input == 'P')
          {
            state_ = http_version_slash;
            return boost::indeterminate;
          }
          else
          {
            return false;
          }
        case http_version_slash:
          if (input == '/')
          {
            req.http_version_major = 0;
            req.http_version_minor = 0;
            state_ = http_version_major_start;
            return boost::indeterminate;
          }
          else
          {
            return false;
          }
        case http_version_major_start:
          if (is_digit(input))
          {
            req.http_version_major = req.http_version_major * 10 + input - '0';
            state_ = http_version_major;
            return boost::indeterminate;
          }
          else
          {
            return false;
          }
        case http_version_major:
          if (input == '.')
          {
            state_ = http_version_minor_start;
            return boost::indeterminate;
          }
          else if (is_digit(input))
          {
            req.http_version_major = req.http_version_major * 10 + input - '0';
            return boost::indeterminate;
          }
          else
          {
            return false;
          }
        case http_version_minor_start:
          if (is_digit(input))
          {
            req.http_version_minor = req.http_version_minor * 10 + input - '0';
            state_ = http_version_minor;
            return boost::indeterminate;
          }
          else
          {
            return false;
          }
        case http_version_minor:
          if (input == '\r')
          {
            state_ = expecting_newline_1;
            return boost::indeterminate;
          }
          else if (is_digit(input))
          {
            req.http_version_minor = req.http_version_minor * 10 + input - '0';
            return boost::indeterminate;
          }
          else
          {
            return false;
          }
        case expecting_newline_1:
          if (input == '\n')
          {
            state_ = header_line_start;
            return boost::indeterminate;
          }
          else
          {
            return false;
          }
        case header_line_start:
          if (input == '\r')
          {
            state_ = expecting_newline_3;
            return boost::indeterminate;
          }
          else if (!req.headers.empty() && (input == ' ' || input == '\t'))
          {
            state_ = header_lws;
            return boost::indeterminate;
          }
          else if (!is_char(input) || is_ctl(input) || is_tspecial(input))
          {
            return false;
          }
          else
          {
            req.headers.push_back(header());
            req.headers.back().name.push_back(input);
            state_ = header_name;
            return boost::indeterminate;
          }
        case header_lws:
          if (input == '\r')
          {
            state_ = expecting_newline_2;
            return boost::indeterminate;
          }
          else if (input == ' ' || input == '\t')
          {
            return boost::indeterminate;
          }
          else if (is_ctl(input))
          {
            return false;
          }
          else
          {
            state_ = header_value;
            req.headers.back().value.push_back(input);
            return boost::indeterminate;
          }
        case header_name:
          if (input == ':')
          {
            state_ = space_before_header_value;
            return boost::indeterminate;
          }
          else if (!is_char(input) || is_ctl(input) || is_tspecial(input))
          {
            return false;
          }
          else
          {
            req.headers.back().name.push_back(input);
            return boost::indeterminate;
          }
        case space_before_header_value:
          if (input == ' ')
          {
            state_ = header_value;
            return boost::indeterminate;
          }
          else
          {
            return false;
          }
        case header_value:
          if (input == '\r')
          {
            state_ = expecting_newline_2;
            return boost::indeterminate;
          }
          else if (is_ctl(input))
          {
            return false;
          }
          else
          {
            req.headers.back().value.push_back(input);
            return boost::indeterminate;
          }
        case expecting_newline_2:
          if (input == '\n')
          {
            state_ = header_line_start;
            return boost::indeterminate;
          }
          else
          {
            return false;
          }
        case expecting_newline_3:
          return (input == '\n');
        default:
          return false;
      }
    }

    bool request_parser::is_char(int c)
    {
      return c >= 0 && c <= 127;
    }

    bool request_parser::is_ctl(int c)
    {
      return (c >= 0 && c <= 31) || (c == 127);
    }

    bool request_parser::is_tspecial(int c)
    {
      switch (c)
      {
        case '(': case ')': case '<': case '>': case '@':
        case ',': case ';': case ':': case '\\': case '"':
        case '/': case '[': case ']': case '?': case '=':
        case '{': case '}': case ' ': case '\t':
          return true;
        default:
          return false;
      }
    }

    bool request_parser::is_digit(int c)
    {
      return c >= '0' && c <= '9';
    }

  } // namespace httpd
} // namespace epidb
