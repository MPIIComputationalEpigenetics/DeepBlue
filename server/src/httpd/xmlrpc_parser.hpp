//
//  xmlrpc_parser.hpp
//  epidb
//
//  Created by Fabian Reinartz on 15.07.13.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#ifndef EPIDB_HTTPD_XMLRPC_PARSER_HPP
#define EPIDB_HTTPD_XMLRPC_PARSER_HPP

#include <string>
#include <vector>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

#include "../extras/serialize.hpp"

#include "../../third_party/expat/lib/expat.h"

#include "xmlrpc_request.hpp"

namespace epidb {
  namespace httpd {

    class XMLRPCParser : private boost::noncopyable {
     public:
      XMLRPCParser();
      ~XMLRPCParser();

      bool parse(char* buf, int len);
      bool done(boost::shared_ptr<XmlrpcRequest>& req);

     private:
      typedef enum { METHOD_CALL, METHOD_NAME, PARAMS, PARAM, TYPE, MEMBER, NAME, VALUE, NONE } State;

      static void start_handler(void *data, const XML_Char *name, const XML_Char **atts);
      static void end_handler(void *data, const XML_Char *name);
      static void char_handler(void *data, const XML_Char *s, int len);

      XML_Parser parser_;
      boost::shared_ptr<XmlrpcRequest> request_;

      std::string buf_;
      std::string last_member_name_;
      std::vector<State> stack_;

      std::vector<serialize::ParameterPtr> params_;

      bool error_;
    };

  } // namespace httpd
} // namespace epidb

#endif // EPIDB_HTTPD_XMLRPC_PARSER_HPP
