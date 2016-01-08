//
//  request_handler.cpp
//  DeepBlue Epigenomic Data Server

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

#include <string>
#include <string.h>

#include <iostream>

#include "request.hpp"
#include "xmlrpc_parser.hpp"

#include "../extras/serialize.hpp"
#include "../extras/xmlrpc.hpp"

#include "../log.hpp"

namespace epidb {
  namespace httpd {

    XMLRPCParser::XMLRPCParser()
      : request_(), error_(false) {
      parser_ = XML_ParserCreate(NULL);

      XML_SetUserData(parser_, this);
      XML_SetElementHandler(parser_, start_handler, end_handler);
      XML_SetCharacterDataHandler(parser_, char_handler);
    }

    XMLRPCParser::~XMLRPCParser() {
      XML_ParserFree(parser_);
    }

    bool XMLRPCParser::parse(char* buf, int len) {
      XML_Parse(parser_, buf, len, 0);
      return !error_;
    }

    bool XMLRPCParser::done(std::shared_ptr<XmlrpcRequest>& req) {
      if (error_) {
        return false;
      }
      if (stack_.size() > 0) {
        EPIDB_LOG_WARN("stack not empty on done\n");
        return false;
      }
      req = request_;
      return true;
    }

    void XMLRPCParser::start_handler(void *data, const XML_Char *name, const XML_Char **atts) {
      XMLRPCParser* self = reinterpret_cast<XMLRPCParser*>(data);
      if (self->error_) return;

      int size = self->stack_.size();
      State last = NONE;
      if (size > 0) {
        last = self->stack_.back();
      }

      self->buf_.clear();

      if (strcmp("value", name) == 0) {
        if ((size != 3 || last != PARAM) && last != TYPE && last != MEMBER) {
          self->error_ = true;
          return;
        }
        self->stack_.push_back(VALUE);
      }
      else if (strcmp("param", name) == 0) {
        if (size != 2 || last != PARAMS) {
          self->error_ = true;
          return;
        }
        self->stack_.push_back(PARAM);
      }
      else if (strcmp("params", name) == 0) {
        if (size != 1 || last != METHOD_CALL) {
          self->error_ = true;
          return;
        }
        self->stack_.push_back(PARAMS);
      }
      else if (strcmp("methodName", name) == 0) {
        if (size != 1 || last != METHOD_CALL) {
          self->error_ = true;
          return;
        }
        self->stack_.push_back(METHOD_NAME);
      }
      else if (strcmp("methodCall", name) == 0) {
        if (size != 0) {
          self->error_ = true;
          return;
        }
        self->stack_.push_back(METHOD_CALL);
      }
      else if (strcmp("data", name) == 0) {
        if (last != TYPE) {
          self->error_ = true;
          return;
        }
      }
      else if (strcmp("member", name) == 0) {
        if (last != TYPE) {
          self->error_ = true;
          return;
        }
        self->stack_.push_back(MEMBER);
      }
      else if (strcmp("name", name) == 0) {
        if (last != MEMBER) {
          self->error_ = true;
          return;
        }
        self->stack_.push_back(NAME);
      }
      // if a value was opened a type has to be next
      else if (last == VALUE) {
        xmlrpc::Type t;
        // check if valid type is opened
        if (!xmlrpc::check_type(name, t)) {
          self->error_ = true;
          return;
        }
        self->stack_.push_back(TYPE);

        // create a new parameter on the param stack
        serialize::ParameterPtr p;
        if (t == xmlrpc::ARRAY) {
          p = serialize::ParameterPtr(new serialize::ListParameter());
        }
        else if (t == xmlrpc::STRUCT) {
          p = serialize::ParameterPtr(new serialize::MapParameter());
        }
        // TODO: Check for all types, if no one match, return error. (longer but safer)
        else {
          p = serialize::ParameterPtr(new serialize::SimpleParameter(serialize::from_xml_type(t)));
        }
        self->params_.push_back(p);
      }
      else {
        self->error_ = true; // unknown tag
      }
    }

    void XMLRPCParser::end_handler(void *data, const XML_Char *name) {
      XMLRPCParser* self = reinterpret_cast<XMLRPCParser*>(data);
      if (self->error_) return;

      if (self->stack_.size() == 0) {
        return;
      }

      State last = self->stack_.back();

      if (strcmp("value", name) == 0) {
        if (last != VALUE) {
          self->error_ = true;
          return;
        }
      }
      else if (strcmp("param", name) == 0) {
        if (last != PARAM) {
          self->error_ = true;
          return;
        }
      }
      else if (strcmp("params", name) == 0) {
        if (last != PARAMS) {
          self->error_ = true;
          return;
        }
      }
      else if (strcmp("methodName", name) == 0) {
        if (last != METHOD_NAME) {
          self->error_ = true;
          return;
        }
        self->request_ = std::shared_ptr<XmlrpcRequest>(new XmlrpcRequest(self->buf_));
        self->buf_.clear();
      }
      else if (strcmp("methodCall", name) == 0) {
        if (last != METHOD_CALL) {
          self->error_ = true;
          return;
        }
      }
      else if (strcmp("data", name) == 0) {
        if (last != TYPE) {
          self->error_ = true;
        }
        return;
      }
      else if (strcmp("member", name) == 0) {
        if (last != MEMBER) {
          self->error_ = true;
          return;
        }
      }
      else if (strcmp("name", name) == 0) {
        if (last != NAME) {
          self->error_ = true;
          return;
        }
        self->last_member_name_ = self->buf_;
        self->buf_.clear();
      }
      // a value's content is finished if the correct last type is closed
      else if (last == TYPE) {
        xmlrpc::Type t;
        if (!xmlrpc::check_type(name, t)) {
          self->error_ = true;
          return;
        }
        // check if correct type is closed
        serialize::ParameterPtr last_param = self->params_.back();
        if (serialize::to_xml_type(last_param->type()) != t) {
          self->error_ = true;
          return;
        }
        // if a primitive type, set the actual value
        if (last_param->type() != serialize::LIST && last_param->type() != serialize::MAP) {
          last_param->set_value(self->buf_);
          self->buf_.clear();
        }
        // remove closed element from param stack
        self->params_.pop_back();
        // if a parent parameter exists, append it
        if (self->params_.size() != 0) {
          serialize::ParameterPtr parent_param = self->params_.back();
          if (parent_param->type() == serialize::LIST) {
            parent_param->add_child(last_param);
          }
          else if (parent_param->type() == serialize::MAP) {
            parent_param->add_child(self->last_member_name_, last_param);
          }
          else {
            self->error_ = true;
          }
        }
        // otherwise hand the top level param to the request
        else {
          if (!self->request_) {
            self->error_ = true;
            return;
          }
          self->request_->params().add_param(last_param);
        }


      } else {
        self->error_ = true;
        return;
      }
      self->stack_.pop_back();
    }

    void XMLRPCParser::char_handler(void *data, const XML_Char *s, int len) {
      XMLRPCParser* self = reinterpret_cast<XMLRPCParser*>(data);

      if (self->error_) return;

      if (self->stack_.size() > 0) {
        State last = self->stack_.back();
        if (last == TYPE || last == METHOD_NAME || last == NAME)
          self->buf_.append(s, len);
      }
    }

  } // namespace httpd
} // namespace epidb
