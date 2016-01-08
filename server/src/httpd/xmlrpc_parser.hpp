//
//  xmlrpc_parser.hpp
//  epidb
//
//  Created by Fabian Reinartz on 15.07.13.
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

#ifndef EPIDB_HTTPD_XMLRPC_PARSER_HPP
#define EPIDB_HTTPD_XMLRPC_PARSER_HPP

#include <string>
#include <vector>
#include <boost/noncopyable.hpp>
#include <memory>

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
      bool done(std::shared_ptr<XmlrpcRequest>& req);

     private:
      typedef enum { METHOD_CALL, METHOD_NAME, PARAMS, PARAM, TYPE, MEMBER, NAME, VALUE, NONE } State;

      static void start_handler(void *data, const XML_Char *name, const XML_Char **atts);
      static void end_handler(void *data, const XML_Char *name);
      static void char_handler(void *data, const XML_Char *s, int len);

      XML_Parser parser_;
      std::shared_ptr<XmlrpcRequest> request_;

      std::string buf_;
      std::string last_member_name_;
      std::vector<State> stack_;

      std::vector<serialize::ParameterPtr> params_;

      bool error_;
    };

  } // namespace httpd
} // namespace epidb

#endif // EPIDB_HTTPD_XMLRPC_PARSER_HPP
