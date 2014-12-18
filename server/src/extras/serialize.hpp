#ifndef EPIDB_EXTRAS_SERIALIZE_HPP_
#define EPIDB_EXTRAS_SERIALIZE_HPP_

#include <string>
#include <vector>
#include <map>
#include <boost/shared_ptr.hpp>

#include "stringbuilder.hpp"

#include "xmlrpc.hpp"

namespace epidb {
  namespace serialize {

    typedef enum {
      INTEGER, STRING, BOOLEAN, DOUBLE, NIL, ERROR, // simple types
      LIST, MAP, DATASTRING, DATABIN
    } Type;

    std::string encode(const std::string& data);

    xmlrpc::Type to_xml_type(const Type t);

    Type from_xml_type(const xmlrpc::Type t);

    std::string type_name(const Type t);

    class Parameter {
     public:

      explicit Parameter();
      virtual ~Parameter();

      virtual const std::string get_xml() const = 0;

      virtual Type type() const = 0;

      virtual bool set_value(const std::string& v);

      virtual void set_type(Type type);

      virtual bool add_child(const boost::shared_ptr<Parameter>& p);

      virtual bool add_child(const std::string& name, const boost::shared_ptr<Parameter>&);

      virtual bool children(std::vector<boost::shared_ptr<Parameter> >& results);

      virtual bool children(std::map<std::string, boost::shared_ptr<Parameter> >& results);

      virtual bool children(std::vector<std::pair<std::string, boost::shared_ptr<Parameter> > >& results);

      virtual const std::string value() const = 0;

      bool isNull() const;
      bool isBool() const;
      bool isString() const;
      bool isDouble() const;
      bool isInt() const;
      bool isNumber() const;
      bool isList() const;
      bool isMap() const;
      bool isDatastring() const;
      bool isDatabin() const;

      const std::string as_string() const;

      double as_double() const;

      long long as_long() const;

      double as_number() const;

      bool as_boolean() const;
    };

    typedef boost::shared_ptr<Parameter> ParameterPtr;


    class SimpleParameter
    : public Parameter
    {
     private:
      Type type_;
      std::string value_;

     public:
      SimpleParameter(const Type& type);
      SimpleParameter(const Type& type, const std::string& value);
      SimpleParameter(const std::string& s);
      SimpleParameter(const bool b);
      SimpleParameter(const long long i);
      SimpleParameter(const double d);

      Type type() const;
      const std::string value() const;

      bool set_value(const std::string& v);

      void set_type(Type type);

      const std::string get_xml() const;
    };

    class ListParameter
    : public Parameter
    {
      std::vector<ParameterPtr> array_;

     public:
      ListParameter();
      ListParameter(const std::vector<ParameterPtr>& array);

      const std::string as_string() const;

      Type type() const;

      const std::string value() const;

      const std::string get_xml() const;

      bool add_child(const ParameterPtr& p);

      bool children(std::vector<ParameterPtr>& results);

      void add_string(std::string str);

      void add_string(int i);

      void add_int(size_t i);
    };

    class MapParameter
    : public Parameter
    {
     private:
      std::map<std::string, ParameterPtr> map_;
      bool ordered_;
      std::vector<std::string> key_order_;

     public:
      MapParameter(bool ordered=true);
      MapParameter(const std::map<std::string, ParameterPtr>& map, bool ordered=true);

      const std::string as_string() const;

      Type type() const;
      const std::string value() const;

      const std::string get_xml() const;

      bool add_child(const std::string& key, const ParameterPtr& p);

      bool children(std::map<std::string, ParameterPtr>& results);

      bool children(std::vector<std::pair<std::string, ParameterPtr> >& results);
    };


    class Parameters {
     private:
      std::vector<ParameterPtr> params_;
      bool as_array_;

     public:
      Parameters();

      typedef std::vector<ParameterPtr>::iterator iterator;
      typedef std::vector<ParameterPtr>::const_iterator const_iterator;

      // indirections for standard vector methods
      size_t size() const { return params_.size(); }
      ParameterPtr& operator[](int i) { return params_[i]; }
      const ParameterPtr& operator[](int i) const { return params_[i]; }

      Parameters::iterator begin() { return params_.begin(); }
      Parameters::iterator end() { return params_.end(); }
      Parameters::const_iterator begin() const { return params_.begin(); }
      Parameters::const_iterator end() const { return params_.end(); }

      const std::vector<ParameterPtr> get() const;

      std::string string(bool truncate_strings=true) const;

      bool as_array() const;

      void set_as_array(bool b);

      void add_error(const std::string& msg);

      void add_string(const std::string& str);

      void add_stringbuilder(StringBuilder &sb);

      void add_string(int i);

      void add_int(size_t i);

      void add_double(double i);

      void add_bool(bool i);

      void add_list(std::vector<serialize::ParameterPtr> list);

      void add_list(ParameterPtr list);

      void add_param(ParameterPtr p);

    };

  } // namespace serialize
} // namespace epidb

#endif // EPIDB_EXTRAS_SERIALIZE_HPP_