#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <boost/shared_ptr.hpp>

#include "utils.hpp"
#include "serialize.hpp"
#include "stringbuilder.hpp"

namespace epidb {
  namespace serialize {

    std::string type_name(const Type t)
    {
      switch (t) {
      case INTEGER:
        return "integer";
      case STRING:
        return "string";
      case BOOLEAN:
        return "boolean";
      case DOUBLE:
        return "double";
      case NIL:
        return "nil";
      case ERROR:
        return "error";
      case LIST:
        return "list";
      case MAP:
        return "map";
      case DATASTRING:
        return "string";
      case DATABIN:
        return "bindata";
      default:
        return "Unknown type: " + utils::integer_to_string(t);
      }
    }

    xmlrpc::Type to_xml_type(const Type t)
    {
      switch (t) {
      case INTEGER:
        return xmlrpc::INTEGER;
      case STRING:
        return xmlrpc::STRING;
      case DATASTRING:
        return xmlrpc::STRING;
      case DATABIN:
        return xmlrpc::BASE64;
      case BOOLEAN:
        return xmlrpc::BOOLEAN;
      case DOUBLE:
        return xmlrpc::DOUBLE;
      case NIL:
        return xmlrpc::NIL;
      case ERROR:
        return xmlrpc::STRING;
      case LIST:
        return xmlrpc::ARRAY;
      default:
        return xmlrpc::STRUCT;
      }
    }

    Type from_xml_type(const xmlrpc::Type t)
    {
      switch (t) {
      case xmlrpc::INTEGER:
        return INTEGER;
      case xmlrpc::BOOLEAN:
        return BOOLEAN;
      case xmlrpc::DOUBLE:
        return DOUBLE;
      case xmlrpc::NIL:
        return NIL;
      case xmlrpc::ARRAY:
        return LIST;
      case xmlrpc::STRUCT:
        return MAP;
      case xmlrpc::BASE64:
        return DATABIN;
      default:
        return STRING;
      }
    }

    Parameter::Parameter() {}
    Parameter::~Parameter() {}

    bool Parameter::set_value(const std::string &v)
    {
      return false;
    }

    void Parameter::set_type(Type type)
    {
      std::cerr << "Nothing." << std::endl;
    }

    bool Parameter::add_child(const boost::shared_ptr<Parameter> &p)
    {
      return false;
    }

    bool Parameter::add_child(const std::string &name, const boost::shared_ptr<Parameter> &)
    {
      return false;
    }

    bool Parameter::children(std::vector<boost::shared_ptr<Parameter> > &results)
    {
      return false;
    }

    bool Parameter::children(std::map<std::string, boost::shared_ptr<Parameter> > &results)
    {
      return false;
    }

    bool Parameter::children(std::vector<std::pair<std::string, ParameterPtr> > &results)
    {
      return false;
    }

    bool Parameter::isNull() const
    {
      return type() == NIL;
    }

    bool Parameter::isBool() const
    {
      return type() == BOOLEAN;
    }

    bool Parameter::isString() const
    {
      return type() == STRING;
    }

    bool Parameter::isDouble() const
    {
      return type() == DOUBLE;
    }

    bool Parameter::isInt() const
    {
      return type() == INTEGER;
    }

    bool Parameter::isNumber() const
    {
      return isInt() || isDouble();
    }

    bool Parameter::isList() const
    {
      return type() == LIST;
    }

    bool Parameter::isMap() const
    {
      return type() == MAP;
    }

    bool Parameter::isDatastring() const
    {
      return type() == DATASTRING;
    }

    bool Parameter::isDatabin() const
    {
      return type() == DATABIN;
    }

    const std::string Parameter::as_string() const
    {
      return value();
    }

    double Parameter::as_double() const
    {
      return atof(as_string().c_str());
    }

    long long Parameter::as_long() const
    {
      return atoi(as_string().c_str());
    }

    double Parameter::as_number() const
    {
      if (isInt()) {
        return as_long();
      }
      return as_double();
    }

    bool Parameter::as_boolean() const
    {
      if (isBool() || isString()) {
        return (value() == "true");
      }
      return as_number() <= 0;
    }

    SimpleParameter::SimpleParameter(const Type &type) :
      type_(type)
    {}

    SimpleParameter::SimpleParameter(const Type &type, const std::string &value) :
      type_(type), value_(utils::sanitize(value))
    {}

    SimpleParameter::SimpleParameter(const std::string &s) :
      type_(STRING), value_(utils::sanitize(s))
    {}

    SimpleParameter::SimpleParameter(const bool b) :
      type_(BOOLEAN), value_(b ? "1" : "0")
    {}

    SimpleParameter::SimpleParameter(const long long i) :
      type_(INTEGER), value_(utils::integer_to_string(i))
    {}

    SimpleParameter::SimpleParameter(const double d) :
      type_(DOUBLE), value_(utils::double_to_string(d))
    {}

    Type SimpleParameter::type() const
    {
      return type_;
    }

    void SimpleParameter::set_type(Type type)
    {
      std::cerr << "mudou" << std::endl;
      std::cerr << type_ << std::endl;
      std::cerr << type << std::endl;
      type_ = type;
    }

    bool SimpleParameter::set_value(const std::string &v)
    {
      value_ = v;
      return true;
    }

    const std::string SimpleParameter::value() const
    {
      if (isBool()) {
        if (value_ == "1")
          return "true";
        if (value_ == "0")
          return "false";
      }
      return value_;
    }

    const std::string SimpleParameter::get_xml() const
    {
      std::stringstream ss;
      std::string ts = xmlrpc::type_string(to_xml_type(type_));
      ss << "<value>";
      ss << "<" << ts << ">";
      ss << value_;
      ss << "</" << ts << ">";
      ss << "</value>";
      return ss.str();
    }

    ListParameter::ListParameter() {}

    ListParameter::ListParameter(const std::vector<ParameterPtr> &array) :
      array_(array)
    {}

    Type ListParameter::type() const
    {
      return LIST;
    }

    const std::string ListParameter::value() const
    {
      return as_string();
    }

    const std::string ListParameter::as_string() const
    {
      std::stringstream ss;
      ss << "[";
      std::vector<ParameterPtr>::const_iterator it;
      for (it = array_.begin(); it != array_.end(); ++it) {
        if (it != array_.begin()) {
          ss << ", ";
        }
        ss << (**it).as_string();
      }
      ss << "]";

      return ss.str();
    }

    const std::string ListParameter::get_xml() const
    {
      std::stringstream ss;
      ss << "<value>";
      ss << "<array>";
      ss << "<data>" << std::endl;
      // add array content
      std::vector<ParameterPtr>::const_iterator it;
      for (it = array_.begin(); it != array_.end(); ++it) {
        ss << (*it)->get_xml() << std::endl;
      }
      ss << "</data>";
      ss << "</array>";
      ss << "</value>";
      return ss.str();
    }

    bool ListParameter::add_child(const ParameterPtr &p)
    {
      array_.push_back(p);
      return true;
    }

    bool ListParameter::children(std::vector<ParameterPtr> &results)
    {
      if (array_.size() == 1 && array_[0]->as_string().empty()) {
        return true;
      }
      results = array_;
      return true;
    }


    MapParameter::MapParameter(bool ordered) : ordered_(ordered) {}

    MapParameter::MapParameter(const std::map<std::string, ParameterPtr> &map, bool ordered) :
      map_(map), ordered_(ordered)
    {
      if (ordered) {
        std::map<std::string, ParameterPtr>::const_iterator it;
        for (it = map_.begin(); it != map_.end(); ++it) {
          key_order_.push_back(utils::sanitize(it->first));
        }
      }
    }

    Type MapParameter::type() const
    {
      return MAP;
    }

    const std::string MapParameter::value() const
    {
      return as_string();
    }

    const std::string MapParameter::as_string() const
    {
      std::stringstream ss;
      ss << "[";
      if (ordered_) {
        std::vector<std::string>::const_iterator it;
        for (it = key_order_.begin(); it != key_order_.end(); ++it) {
          if (it != key_order_.begin()) {
            ss << ", ";
          }
          ss << *it << ":" << (*(const_cast<std::map<std::string, ParameterPtr> *>(&map_)))[*it]->as_string();
        }
      } else {
        std::map<std::string, ParameterPtr>::const_iterator it;
        for (it = map_.begin(); it != map_.end(); ++it) {
          if (it != map_.begin()) {
            ss << ", ";
          }
          ss << it->first << ":" << it->second->as_string();
        }
      }
      ss << "]";

      return ss.str();
    }

    const std::string MapParameter::get_xml() const
    {
      std::stringstream ss;
      ss << "<value>";
      ss << "<struct>" << std::endl;

      if (ordered_) {
        // add array content
        std::vector<std::string>::const_iterator it;
        for (it = key_order_.begin(); it != key_order_.end(); ++it) {
          ss << "<member>";
          ss << "<name>" << *it << "</name>";
          ss << (*(const_cast<std::map<std::string, ParameterPtr> *>(&map_)))[*it]->get_xml();
          ss << "</member>" << std::endl;
        }
      } else {
        std::map<std::string, ParameterPtr>::const_iterator it;
        for (it = map_.begin(); it != map_.end(); ++it) {
          ss << "<member>";
          ss << "<name>" << it->first << "</name>";
          ss << it->second->get_xml();
          ss << "</member>" << std::endl;
        }
      }

      ss << "</struct>";
      ss << "</value>";
      return ss.str();
    }

    bool MapParameter::add_child(const std::string &key, const ParameterPtr &p)
    {
      std::string s_key = utils::sanitize(key);
      if (ordered_) {
        key_order_.push_back(s_key);
      }
      map_[s_key] = p;
      return true;
    }

    bool MapParameter::children(std::map<std::string, ParameterPtr> &results)
    {
      results = map_;
      return true;
    }

    bool MapParameter::children(std::vector<std::pair<std::string, ParameterPtr> > &results)
    {
      if (ordered_) {
        std::vector<std::string>::const_iterator it;
        for (it = key_order_.begin(); it != key_order_.end(); ++it) {
          results.push_back(std::pair<std::string, ParameterPtr>(*it, map_[*it]));
        }
      } else {
        std::map<std::string, ParameterPtr>::const_iterator it;
        for (it = map_.begin(); it != map_.end(); ++it) {
          results.push_back(std::pair<std::string, ParameterPtr>(utils::sanitize(it->first), it->second));
        }
      }
      return true;
    }


    Parameters::Parameters() : as_array_(false) {}

    const std::vector<ParameterPtr> Parameters::get() const
    {
      return params_;
    }

    std::string Parameters::string(bool truncate_strings) const
    {
      std::stringstream params_ss;
      for (const_iterator it = begin(); it != end(); ++it) {
        if (it != begin()) {
          params_ss << ", ";
        }
        if ((**it).isDatastring()) {
          params_ss << "<string data>";
        } else if ((**it).isNull()) {
          params_ss << "<null>";
        } else if ((**it).isString() && truncate_strings) {
          std::string s = (**it).as_string();
          if (s.length() > 1024) {
            params_ss << s.substr(0, 1024);
            params_ss << " ... [" << s.length() - 1024 << " more characters]";
          } else {
            params_ss << s;
          }
        } else {
          if (truncate_strings) {
            std::string tmp_string((**it).as_string());
            size_t l = tmp_string.length();
            if (l > 1024) {
              params_ss << tmp_string.substr(0, 1024);
              params_ss << " ... [" << l - 1024 << " more characters]";
            }
          } else {
            params_ss << (**it).as_string();
          }
        }
      }
      return params_ss.str();
    }

    bool Parameters::as_array() const
    {
      return as_array_;
    }

    void Parameters::set_as_array(bool b)
    {
      as_array_ = b;
    }

    void Parameters::add_error(const std::string &msg)
    {
      params_.push_back(ParameterPtr(new SimpleParameter(ERROR, msg)));
    }

    void Parameters::add_string(const std::string &str)
    {
      params_.push_back(ParameterPtr(new SimpleParameter(STRING, utils::sanitize(str))));
    }

    void Parameters::add_string(int i)
    {
      std::string value = utils::integer_to_string(i);
      params_.push_back(ParameterPtr(new SimpleParameter(STRING, value)));
    }

    void Parameters::add_stringbuilder(StringBuilder &sb)
    {
      // TODO: In fact, read the file content only when build the xml response.
      params_.push_back(ParameterPtr(new SimpleParameter(STRING, sb.to_string())));
    }

    void Parameters::add_int(size_t i)
    {
      std::string value = utils::integer_to_string(i);
      params_.push_back(ParameterPtr(new SimpleParameter(INTEGER, value)));
    }

    void Parameters::add_double(double d)
    {
      std::string value = utils::double_to_string(d);
      params_.push_back(ParameterPtr(new SimpleParameter(DOUBLE, value)));
    }

    void Parameters::add_list(ParameterPtr list)
    {
      params_.push_back(list);
    }

    void Parameters::add_list(std::vector<serialize::ParameterPtr> list)
    {
      params_.push_back(ParameterPtr(new ListParameter(list)));
    }

    void Parameters::add_bool(bool b)
    {
      std::string value = "1";
      if (!b) {
        value = "0";
      }
      params_.push_back(ParameterPtr(new SimpleParameter(BOOLEAN, value)));
    }

    void Parameters::add_param(ParameterPtr p)
    {
      params_.push_back(p);
    }

  } // namespace serialize
} // namespace epidb
