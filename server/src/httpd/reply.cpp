//
//  header.hpp
//  epidb
//
//  Created by Felipe Albrecht on 27.05.13.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//
//
// based on:
//
// reply.cpp
// ~~~~~~~~~
//
// Copyright (c) 2003-2012 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <string>

#include "reply.hpp"

#include "extras/utils.hpp"

namespace epidb {
  namespace httpd {

    namespace StatusStrings {

      const std::string ok =
      "HTTP/1.0 200 OK\r\n";
      const std::string created =
      "HTTP/1.0 201 Created\r\n";
      const std::string accepted =
      "HTTP/1.0 202 Accepted\r\n";
      const std::string no_content =
      "HTTP/1.0 204 No Content\r\n";
      const std::string multiple_choices =
      "HTTP/1.0 300 Multiple Choices\r\n";
      const std::string moved_permanently =
      "HTTP/1.0 301 Moved Permanently\r\n";
      const std::string moved_temporarily =
      "HTTP/1.0 302 Moved Temporarily\r\n";
      const std::string not_modified =
      "HTTP/1.0 304 Not Modified\r\n";
      const std::string bad_request =
      "HTTP/1.0 400 Bad Request\r\n";
      const std::string unauthorized =
      "HTTP/1.0 401 Unauthorized\r\n";
      const std::string forbidden =
      "HTTP/1.0 403 Forbidden\r\n";
      const std::string not_found =
      "HTTP/1.0 404 Not Found\r\n";
      const std::string internal_server_error =
      "HTTP/1.0 500 Internal Server Error\r\n";
      const std::string not_implemented =
      "HTTP/1.0 501 Not Implemented\r\n";
      const std::string bad_gateway =
      "HTTP/1.0 502 Bad Gateway\r\n";
      const std::string service_unavailable =
      "HTTP/1.0 503 Service Unavailable\r\n";

      boost::asio::const_buffer to_buffer(Reply::ReplyType status)
      {
        switch (status)
        {
          case Reply::ok:
            return boost::asio::buffer(ok);
          case Reply::created:
            return boost::asio::buffer(created);
          case Reply::accepted:
            return boost::asio::buffer(accepted);
          case Reply::no_content:
            return boost::asio::buffer(no_content);
          case Reply::multiple_choices:
            return boost::asio::buffer(multiple_choices);
          case Reply::moved_permanently:
            return boost::asio::buffer(moved_permanently);
          case Reply::moved_temporarily:
            return boost::asio::buffer(moved_temporarily);
          case Reply::not_modified:
            return boost::asio::buffer(not_modified);
          case Reply::bad_request:
            return boost::asio::buffer(bad_request);
          case Reply::unauthorized:
            return boost::asio::buffer(unauthorized);
          case Reply::forbidden:
            return boost::asio::buffer(forbidden);
          case Reply::not_found:
            return boost::asio::buffer(not_found);
          case Reply::internal_server_error:
            return boost::asio::buffer(internal_server_error);
          case Reply::not_implemented:
            return boost::asio::buffer(not_implemented);
          case Reply::bad_gateway:
            return boost::asio::buffer(bad_gateway);
          case Reply::service_unavailable:
            return boost::asio::buffer(service_unavailable);
          default:
            return boost::asio::buffer(internal_server_error);
        }
      }

    } // namespace status_strings

    namespace misc_strings {

      const char name_value_separator[] = { ':', ' ' };
      const char crlf[] = { '\r', '\n' };

    } // namespace misc_strings

    std::vector<boost::asio::const_buffer> Reply::to_buffers()
    {
      std::vector<boost::asio::const_buffer> buffers;
      buffers.push_back(StatusStrings::to_buffer(type));
      for (std::size_t i = 0; i < headers.size(); ++i)
      {
        header& h = headers[i];
        buffers.push_back(boost::asio::buffer(h.name));
        buffers.push_back(boost::asio::buffer(misc_strings::name_value_separator));
        buffers.push_back(boost::asio::buffer(h.value));
        buffers.push_back(boost::asio::buffer(misc_strings::crlf));
      }
      buffers.push_back(boost::asio::buffer(misc_strings::crlf));
      buffers.push_back(boost::asio::buffer(content));
      return buffers;
    }

    namespace stock_replies {

      const char ok[] = "";
      const char created[] =
      "<html>"
      "<head><title>Created</title></head>"
      "<body><h1>201 Created</h1></body>"
      "</html>";
      const char accepted[] =
      "<html>"
      "<head><title>Accepted</title></head>"
      "<body><h1>202 Accepted</h1></body>"
      "</html>";
      const char no_content[] =
      "<html>"
      "<head><title>No Content</title></head>"
      "<body><h1>204 Content</h1></body>"
      "</html>";
      const char multiple_choices[] =
      "<html>"
      "<head><title>Multiple Choices</title></head>"
      "<body><h1>300 Multiple Choices</h1></body>"
      "</html>";
      const char moved_permanently[] =
      "<html>"
      "<head><title>Moved Permanently</title></head>"
      "<body><h1>301 Moved Permanently</h1></body>"
      "</html>";
      const char moved_temporarily[] =
      "<html>"
      "<head><title>Moved Temporarily</title></head>"
      "<body><h1>302 Moved Temporarily</h1></body>"
      "</html>";
      const char not_modified[] =
      "<html>"
      "<head><title>Not Modified</title></head>"
      "<body><h1>304 Not Modified</h1></body>"
      "</html>";
      const char bad_request[] =
      "<html>"
      "<head><title>Bad Request</title></head>"
      "<body><h1>400 Bad Request</h1></body>"
      "</html>";
      const char unauthorized[] =
      "<html>"
      "<head><title>Unauthorized</title></head>"
      "<body><h1>401 Unauthorized</h1></body>"
      "</html>";
      const char forbidden[] =
      "<html>"
      "<head><title>Forbidden</title></head>"
      "<body><h1>403 Forbidden</h1></body>"
      "</html>";
      const char not_found[] =
      "<html>"
      "<head><title>Not Found</title></head>"
      "<body><h1>404 Not Found</h1></body>"
      "</html>";
      const char internal_server_error[] =
      "<html>"
      "<head><title>Internal Server Error</title></head>"
      "<body><h1>500 Internal Server Error</h1></body>"
      "</html>";
      const char not_implemented[] =
      "<html>"
      "<head><title>Not Implemented</title></head>"
      "<body><h1>501 Not Implemented</h1></body>"
      "</html>";
      const char bad_gateway[] =
      "<html>"
      "<head><title>Bad Gateway</title></head>"
      "<body><h1>502 Bad Gateway</h1></body>"
      "</html>";
      const char service_unavailable[] =
      "<html>"
      "<head><title>Service Unavailable</title></head>"
      "<body><h1>503 Service Unavailable</h1></body>"
      "</html>";

      std::string to_string(Reply::ReplyType status)
      {
        switch (status)
        {
          case Reply::ok:
            return ok;
          case Reply::created:
            return created;
          case Reply::accepted:
            return accepted;
          case Reply::no_content:
            return no_content;
          case Reply::multiple_choices:
            return multiple_choices;
          case Reply::moved_permanently:
            return moved_permanently;
          case Reply::moved_temporarily:
            return moved_temporarily;
          case Reply::not_modified:
            return not_modified;
          case Reply::bad_request:
            return bad_request;
          case Reply::unauthorized:
            return unauthorized;
          case Reply::forbidden:
            return forbidden;
          case Reply::not_found:
            return not_found;
          case Reply::internal_server_error:
            return internal_server_error;
          case Reply::not_implemented:
            return not_implemented;
          case Reply::bad_gateway:
            return bad_gateway;
          case Reply::service_unavailable:
            return service_unavailable;
          default:
            return internal_server_error;
        }
      }

    } // namespace stock_replies

    // TODO: get Access-Control-Allow-Origin from a configuration/option
    Reply Reply::stock_reply(Reply::ReplyType status, std::string&& content)
    {
      Reply rep;
      rep.type = status;

      if (status != ok && content.length() == 0) {
        rep.content = stock_replies::to_string(status);
      } else {
        rep.content = std::move(content);
      }
      rep.headers.resize(3);
      rep.headers[0].name = "content-type";
      rep.headers[0].value = "application/xml";
      rep.headers[1].name = "Access-Control-Allow-Origin";
      rep.headers[1].value = "*";
      rep.headers[2].name = "Content-Length";
      rep.headers[2].value = utils::size_t_to_string(rep.content.size());

      return rep;
    }

    Reply Reply::stock_reply_download(Reply::ReplyType status, const std::string& file_name, std::string&& content)
    {
      Reply rep;
      rep.type = status;

      if (status != ok && content.length() == 0) {
        rep.content = stock_replies::to_string(status);
      } else {
        rep.content = std::move(content);
      }
      rep.headers.resize(4);
      // header('Content-Disposition: ");
      rep.headers[0].name = "content-type";
      rep.headers[0].value = "application/x-bzip2";
      rep.headers[1].name = "Access-Control-Allow-Origin";
      rep.headers[1].value = "*";
      rep.headers[2].name = "Content-Disposition";
      rep.headers[2].value = "attachment; filename=deepblue_data_"+file_name+".bed";
      rep.headers[3].name = "Content-Length";
      rep.headers[3].value = utils::size_t_to_string(rep.content.size());

      return rep;
    }

    // TODO: get Access-Control-Allow-Origin from a configuration/option
    Reply Reply::options_reply()
    {
      Reply rep;
      rep.type = ok;
      rep.headers.resize(4);
      rep.headers[0].name = "Allow";
      rep.headers[0].value = "OPTIONS, POST";
      rep.headers[1].name = "Access-Control-Allow-Origin";
      rep.headers[1].value = "*";
      rep.headers[2].name = "Access-Control-Allow-Headers";
      rep.headers[2].value = "X-Requested-With, Content-Type";
      rep.headers[3].name = "Content-Length";
      rep.headers[3].value = "0";
      return rep;
    }

  } // namespace httpd
} // namespace epidb
