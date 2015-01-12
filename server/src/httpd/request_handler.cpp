//
//  request_handler.cpp
//  epidb
//
//  Created by Felipe Albrecht on 27.05.13.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//
//
// based on:
//
// request_handler.cpp
// ~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2012 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "network.hpp"
#include "reply.hpp"
#include "request.hpp"
#include "xmlrpc_parser.hpp"
#include "xmlrpc_request.hpp"

#include "request_handler.hpp"

#include "log.hpp"

namespace epidb {
  namespace httpd {

    Reply request_handler::handle_request(const Request& req, std::string& content)
    {
      Reply reply;
      if (req.method == "GET")
      {
        reply = Reply::stock_reply(Reply::bad_request, "");
      }

      else if (req.method == "OPTIONS")
      {
        reply = Reply::options_reply();
      }

      else if (req.method == "POST")
      {
        bool error = true;
        std::string message = process_content(req, content, error);

        if (error) {
          reply = Reply::stock_reply(Reply::bad_request, std::move(message));
        } else {
          reply = Reply::stock_reply(Reply::ok, std::move(message));
        }
      }
      return reply;
    }

    const std::string request_handler::process_content(
        const Request& request, std::string& content, bool& error)
    {
      XMLRPCParser parser;
      boost::shared_ptr<XmlrpcRequest> xmlrpc_request;

      char buf[8192];
      size_t len = 0;

      for (size_t i = 0; i < content.length(); ++i){
        buf[len++] = content[i];
        bool done = (i+1) == content.length();
        if (len == 8192 || done) {
          if (!parser.parse(buf, len)) {
            error = true;
            EPIDB_LOG("[1] request from " << request.ip << ": parsing error.");
            return XmlrpcResponse::error_response("[1] parsing error");
          }
          if (done) {
            if(!parser.done(xmlrpc_request)) {
              error = true;
              EPIDB_LOG("[2] request from " << request.ip << ": parsing error.");
              return XmlrpcResponse::error_response("[2] parsing error on done");
            }
            break;
          }
          len = 0;
        }
      }

      if (!xmlrpc_request) {
        EPIDB_LOG("[3] request from " << request.ip << ": parsing error.");
        return XmlrpcResponse::error_response("[3] Error parsing the request data.");
      }

      xmlrpc_request->set_ip(request.ip);
      xmlrpc_request->set_id(request.id_);

      XmlrpcResponse xmlrpc_response(xmlrpc_request->method_name());
      if (!XmlrpcRequestHandler::xmlrpc_request_handle(*xmlrpc_request, xmlrpc_response)) {
        error = true;
        return XmlrpcResponse::error_response("failed request handle");
      }

      error = false;
      return xmlrpc_response.buffer();
    }

  } // namespace httpd
} // namespace epidb
