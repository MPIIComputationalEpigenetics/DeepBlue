//
//  xmlrpc_request.h
//  epidb
//
//  Created by Felipe Albrecht on 27.05.13.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//
#ifndef EPIDB_HTTPD_XMLRPC_REQUEST_HPP
#define EPIDB_HTTPD_XMLRPC_REQUEST_HPP

#include <string>
#include <sstream>

#include "../extras/serialize.hpp"
#include "httpd_exceptions.hpp"

namespace epidb {
  namespace httpd {

    class XmlrpcRequest
    {
     private:
      std::string ip_;
      unsigned long long id_;
      std::string method_name_;
      serialize::Parameters params_;

     public:
      XmlrpcRequest(const std::string& method_name, const std::string& ip)
          : ip_(ip),
            method_name_(method_name)
      {}

      XmlrpcRequest(const std::string& method_name)
          : method_name_(method_name)
      {}

      const std::string& method_name() const {
        return method_name_;
      }

      const std::string& ip() const {
        return ip_;
      }

      void set_ip(const std::string& ip) {
        ip_ = ip;
      }

      unsigned long long id() {
        return id_;
      }

      void set_id(const unsigned long long id) {
        id_ = id;
      }

      serialize::Parameters& params() {
        return params_;
      }

      const serialize::Parameters& params() const {
        return params_;
      }

    };

    class XmlrpcResponse {
     private:

      std::string method_name_;
      serialize::Parameters parameters_;

      static std::string message_header(const std::string& method_name);
      static std::string message_tail();

     public:

      XmlrpcResponse(const std::string& method_name) :
        method_name_(method_name)
      {}

      static const std::string error_response(const std::string& error);

      serialize::Parameters& parameters() {
        return parameters_;
      }

      const std::string buffer() const;
    };


    class XmlrpcRequestHandler {
     public:
      static bool xmlrpc_request_handle(XmlrpcRequest& request, XmlrpcResponse& response);
    };

  } // namespace httpd
} // namespace epidb

#endif // EPIDB_HTTPD_XMLRPC_REQUEST_HPP
