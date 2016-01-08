//
//  xmlrpc.hpp
//  epidb
//
//  Created by Felipe Albrecht on 27.06.13.
//  Copyright (c) 2016 Max Planck Institute for Informatics. All rights reserved.
//

#ifndef EPIDB_EXTRAS_XMLRPC_HPP
#define EPIDB_EXTRAS_XMLRPC_HPP

#include <string>

namespace epidb {
  namespace xmlrpc {

    typedef enum {
      INTEGER, STRING, BASE64, BOOLEAN, DATETIME, DOUBLE, NIL, // simple types
      ARRAY, STRUCT // complex types
    } Type;


    bool check_type(const std::string& name, Type& t);

    const std::string type_string(const Type& t);

  } // namespace xmlrpc
} // namespace epidb

#endif // EPIDB_EXTRAS_XMLRPC_HPP