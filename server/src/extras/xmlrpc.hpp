#ifndef EPIDB_EXTRAS_XMLRPC_HPP_
#define EPIDB_EXTRAS_XMLRPC_HPP_

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

#endif // EPIDB_EXTRAS_XMLRPC_HPP_