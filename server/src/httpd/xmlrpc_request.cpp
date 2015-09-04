//
//  xmlrpc_request.cpp
//  epidb
//
//  Created by Felipe Albrecht on 03.06.13.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//
#include <exception>

#include <boost/shared_ptr.hpp>
#include <boost/foreach.hpp>

#include "xmlrpc_request.hpp"

#include "../engine/engine.hpp"
#include "../errors.hpp"
#include "../log.hpp"

namespace epidb {
  namespace httpd {

    bool XmlrpcRequestHandler::xmlrpc_request_handle(XmlrpcRequest& request,
          XmlrpcResponse& response)
    {
      serialize::Parameters result;
      serialize::Parameters params = request.params();
      bool okay;
      try {
        okay = epidb::Engine::instance().execute(request.method_name(), request.ip(), request.id(), params, result);
      }

      catch (const mongo::UserException& e) {
        std::string err = Error::m(ERR_DATABASE_EXCEPTION, request.method_name(), e.what());
        EPIDB_LOG_ERR(err);
        okay = false;
        // overwrite with clear result
        result = serialize::Parameters();
        result.add_string(err);
      }
      catch (const std::exception& e) {
        std::string s(e.what());
        EPIDB_LOG_ERR("exception on command " << request.method_name() << ": " << s);
        okay = false;
        // overwrite with clear result
        result = serialize::Parameters();
        result.add_string("internal server error" + s);

      } catch (const std::string& ex) {
        EPIDB_LOG_ERR("exception on command " << request.method_name() << ": " << ex);
        okay = false;
        // overwrite with clear result
        result = serialize::Parameters();
        result.add_string("internal server error" + ex);
      }

      if (okay) {
        response.parameters().add_string("okay");
      } else {
        response.parameters().add_string("error");
      }

      if (result.size() == 1 && !result.as_array()) {
        response.parameters().add_param(result[0]);
      }
      else if (result.size() > 0 || result.as_array()) {
        response.parameters().add_list(result.get());
      }

      return true;
    }


    std::string XmlrpcResponse::message_header() {
      std::stringstream m;

      m << "<?xml version=\"1.0\"?>" << "\r\n";
      m << "<methodResponse>" << "\r\n";
      m << "<params>" << "\r\n";

      return m.str();
    }

    std::string XmlrpcResponse::message_tail() {
      std::stringstream m;

      m << "</params>" << "\r\n";
      m << "</methodResponse>" << "\r\n";

      return m.str();
    }

    const std::string XmlrpcResponse::error_response(const std::string& error) {
      std::stringstream m;
      m << message_header();
      m << serialize::SimpleParameter(serialize::ERROR, error).get_xml();
      m << message_tail();
      return m.str();
    }

    const std::string XmlrpcResponse::buffer() const {
      std::stringstream m;

      m << message_header();
      m << "<param>" << std::endl;
      m << "<value>" << std::endl;
      m << "<array>" << std::endl;
      m << "<data>" << std::endl;

      BOOST_FOREACH(const serialize::ParameterPtr& p,  parameters_)
      {
        m << p->get_xml() << std::endl;
      }

      m << "</data>" << std::endl;
      m << "</array>" << std::endl;
      m << "</value>" << std::endl;
      m << "</param>" << std::endl;
      m << message_tail();

      return m.str();
    }

  } // namespace httpd
} // namespace epidb
