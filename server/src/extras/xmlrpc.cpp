#include "xmlrpc.hpp"

namespace epidb {
  namespace xmlrpc {

    bool check_type(const std::string& name, Type& t) {
      if (name.compare("string") == 0) {
        t = STRING;
      }
      else if (name.compare("int") == 0 || name.compare("i4") == 0) {
        t = INTEGER;
      }
      else if (name.compare("base64") == 0) {
        t = BASE64;
      }
      else if (name.compare("nil") == 0) {
        t = NIL;
      }
      else if (name.compare("double") == 0) {
        t = DOUBLE;
      }
      else if (name.compare("boolean") == 0) {
        t = BOOLEAN;
      }
      else if (name.compare("datetime.iso8601") == 0) {
        t = DATETIME;
      }
      else if (name.compare("array") == 0) {
        t = ARRAY;
      }
      else if (name.compare("struct") == 0) {
        t = STRUCT;
      }
      else {
        return false;
      }
      return true;
    }

    const std::string type_string(const Type& t)  {
      if (t == STRING) {
        return "string";
      }
      else if (t == INTEGER) {
        return "int";
      }
      else if (t == BASE64) {
        return "base64";
      }
      else if (t == NIL) {
        return "nil";
      }
      else if (t == DOUBLE) {
        return "double";
      }
      else if (t == BOOLEAN) {
        return "boolean";
      }
      else if (t == DATETIME) {
        return "datetime.iso8601";
      }
      else if (t == ARRAY) {
        return "array";
      }
      else if (t == STRUCT) {
        return "struct";
      }
      return "";
    }

  } // namespace xmlrpc
} // namespace epidb