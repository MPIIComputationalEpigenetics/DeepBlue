//
//  xmlrpc_request.h
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
#ifndef EPIDB_HTTPD_XMLRPC_REQUEST_HPP
#define EPIDB_HTTPD_XMLRPC_REQUEST_HPP

#include <string>
#include <sstream>

#include "../extras/serialize.hpp"

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
            id_(0),
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

      static std::string message_header();
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
