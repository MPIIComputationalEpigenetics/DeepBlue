//
//  utils.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 02.07.13.
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
#include "utils.hpp"

#include <algorithm>
#include <climits>
#include <cctype>
#include <ctime>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>

#include <format.h>

#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>

#include "../log.hpp"
#include "../types.hpp"

namespace epidb {
  namespace utils {

    std::ostream &operator<<(std::ostream &os, const IdName &o)
    {
      os << "[" << o.id << "," << o.name << "]";
      return os;
    }

    IdName bson_to_id_name(const mongo::BSONObj& bson)
    {
      return utils::IdName(bson["_id"].str(), bson["name"].str());
    }


    // Remove it and use the bson to parameters directly
    std::vector<utils::IdName> bsons_to_id_names(const std::vector<mongo::BSONObj> &bsons)
    {
      std::vector<utils::IdName> v;
      for (const mongo::BSONObj & o : bsons) {
        v.push_back(bson_to_id_name(o));
      }
      return v;
    }

    std::ostream &operator<<(std::ostream &os, const IdNameCount &o)
    {
      os << "[" << o.id << "," << o.name << "," << o.count << "]";
      return os;
    }

    std::vector<std::string> capitalize_vector(std::vector<std::string> vector)
    {
      std::transform(vector.begin(), vector.end(), vector.begin(),
      [](std::string s) {
        if (s.empty()) {
          return s;
        }
        s[0] = std::toupper(s[0]);
        return s;
      });

      return vector;
    }

    std::string lower_case(const std::string string)
    {
      std::string data(string);
      std::transform(data.begin(), data.end(), data.begin(), ::tolower);
      return data;
    }

    static uint64_t decdigits[100] = {
      0ll, 0ll, 0ll, 0ll, 0ll, 0ll, 0ll, 0ll, 0ll, 0,
      1ll, 10ll, 100ll, 1000ll, 10000ll, 100000ll, 1000000ll, 10000000ll, 100000000ll, 1000000000ll,
      2ll, 20ll, 200ll, 2000ll, 20000ll, 200000ll, 2000000ll, 20000000ll, 200000000ll, 2000000000ll,
      3ll, 30ll, 300ll, 3000ll, 30000ll, 300000ll, 3000000ll, 30000000ll, 300000000ll, 3000000000ll,
      4ll, 40ll, 400ll, 4000ll, 40000ll, 400000ll, 4000000ll, 40000000ll, 400000000ll, 400000000ll,
      5ll, 50ll, 500ll, 5000ll, 50000ll, 500000ll, 5000000ll, 50000000ll, 500000000ll, 5000000000ll,
      6ll, 60ll, 600ll, 6000ll, 60000ll, 600000ll, 6000000ll, 60000000ll, 600000000ll, 6000000000ll,
      7ll, 70ll, 700ll, 7000ll, 70000ll, 700000ll, 7000000ll, 70000000ll, 700000000ll, 7000000000ll,
      8ll, 80ll, 800ll, 8000ll, 80000ll, 800000ll, 8000000ll, 80000000ll, 800000000ll, 8000000000ll,
      9ll, 90ll, 900ll, 9000ll, 90000ll, 900000ll, 9000000ll, 90000000ll, 900000000ll, 9000000000ll
    };

    template<typename T>
    bool string_to_fixed_point(const std::string &s, T &i)
    {
      const char *p = s.c_str();
      bool neg((*p == '-') ? 1 : 0);
      T num(0);
      size_t pos(strlen(p + neg) - 1);

      if (pos > std::numeric_limits<T>::digits10) {
        return false;
      }

      if (neg)  {
        ++p;
      }
      T prev(0);
      while (*p) {
        if (!std::isdigit(*p)) {
          return false;
        }
        num += decdigits[(*p++ - '0') * 10 + pos--];
        // Check for overflow
        if (num < prev) {
          return false;
        }
        prev = num;
      }
      i = (neg ? -num : num);
      return true;
    }

    bool string_to_int(const std::string &s, int &i)
    {
      return string_to_fixed_point<int>(s, i);
    }

    bool string_to_long(const std::string &s, size_t &i)
    {
      return string_to_fixed_point<size_t>(s, i);
    }

    bool string_to_position(const std::string &s, Position &p)
    {
      return string_to_fixed_point<Position>(s, p);
    }

    template<typename T>
    bool string_to_floating_point(T &r, const char *p)
    {
      // Skip leading white space, if any.
      while (std::isspace(*p) ) {
        p += 1;
      }

      r = std::numeric_limits<T>::min();
      int c = 0; // counter to check how many numbers we got!

      // Get the sign!
      bool neg = false;
      if (*p == '-') {
        neg = true;
        ++p;
      } else if (*p == '+') {
        neg = false;
        ++p;
      }

      // Get the digits before decimal point
      while (std::isdigit(*p)) {
        r = (r * 10.0) + (*p - '0');
        ++p; ++c;
      }

      // Get the digits after decimal point
      if (*p == '.') {
        T f = 0.0;
        T scale = 1.0;
        ++p;
        while (*p >= '0' && *p <= '9') {
          f = (f * 10.0) + (*p - '0');
          ++p;
          scale *= 10.0;
          ++c;
        }
        r += f / scale;
      }

      // FIRST CHECK:
      if (c == 0) {
        return false; // we got no dezimal places! this cannot be any number!
      }


      // Get the digits after the "e"/"E" (exponenet)
      if (*p == 'e' || *p == 'E') {
        int e = 0;

        bool negE = false;
        ++p;
        if (*p == '-') {
          negE = true;
          ++p;
        } else if (*p == '+') {
          negE = false;
          ++p;
        }
        // Get exponent
        c = 0;
        while (std::isdigit(*p)) {
          e = (e * 10) + (*p - '0');
          ++p; ++c;
        }
        if ( !neg && e > std::numeric_limits<T>::max_exponent10 ) {
          e = std::numeric_limits<T>::max_exponent10;
        } else if (neg && e > std::numeric_limits<T>::min_exponent10 ) {
          e = std::numeric_limits<T>::max_exponent10;
        }
        // SECOND CHECK:
        if (c == 0) {
          return false; // we got no  exponent! this was not intended!!
        }

        T scaleE = 1.0;
        // Calculate scaling factor.

        while (e >= 50) {
          scaleE *= 1E50;
          e -= 50;
        }
        //while (e >=  8) { scaleE *= 1E8;  e -=  8; }
        while (e >   0) {
          scaleE *= 10.0;
          e -=  1;
        }

        if (negE) {
          r /= scaleE;
        } else {
          r *= scaleE;
        }
      }

      // POST CHECK:
      // skip post whitespaces
      while ( std::isspace(*p) ) {
        ++p;
      }
      if (*p != '\0') {
        return false; // if next character is not the terminating character
      }

      // Apply sign to number
      if (neg) {
        r = -r;
      }

      return true;
    }

    bool string_to_score(const std::string &s_, Score &c)
    {
      return string_to_floating_point<Score>(c, s_.c_str());
    }

    bool string_to_double(const std::string &s_, double &c)
    {
      return string_to_floating_point<double>(c, s_.c_str());
    }

    std::string score_to_string(const Score s)
    {
      return fmt::format("{:-.4f}", s);
    }

    std::string size_t_to_string(const size_t t)
    {
      return fmt::FormatInt(t).str();
    }

    std::string integer_to_string(const int t)
    {
      return fmt::FormatInt(t).str();
    }

    std::string long_to_string(const long t)
    {
      return fmt::FormatInt(t).str();
    }

    bool is_number(const std::string &s_)
    {
      std::string ss(s_);
      boost::trim(ss);
      return !ss.empty() && std::find_if(ss.begin(), ss.end(), [](char c) {
        return !std::isdigit(c);
      }) == ss.end();
    }

    std::string lower(const std::string &in)
    {
      std::string data(in);
      std::transform(data.begin(), data.end(), data.begin(), ::tolower);
      return data;
    }

    std::string normalize_name(const std::string &in)
    {
      std::string tmp(in);
      boost::trim(tmp);

      std::string out;

      for (std::string::const_iterator it = tmp.begin(); it != tmp.end(); ++it) {
        char c = *it;
        if (isalnum(c)) {
          out += tolower(c);
        } if (c == '+') {
          out += c;
        }
      }
      return out;
    }

    std::string normalize_annotation_name(const std::string &annotation_name)
    {
      std::string tmp(annotation_name);
      boost::trim(tmp);

      std::string out;

      for (std::string::const_iterator it = tmp.begin(); it != tmp.end(); ++it) {
        char c = *it;
        if (isspace(c)) {
          continue;
        }
        if (isalnum(c)) {
          out += tolower(c);
        } else {
          out += c;
        }
      }
      return out;
    }

    std::string normalize_epigenetic_mark(const std::string &epigenetic_mark)
    {
      const std::string l = normalize_name(epigenetic_mark);

      // Norm histone modifications
      boost::regex expression("h([134]|2[ab])([a-z])([[:digit:]]+)(.*)");
      boost::cmatch what;

      if (boost::regex_match(l.c_str(), what, expression)) {
        std::string histone(what[1].first, what[1].second - what[1].first);
        std::string aa(what[2].first, what[2].second - what[2].first);
        std::string position(what[3].first, what[3].second - what[3].first);
        std::string rest(what[4].first, what[4].second - what[4].first);
        size_t value;
        if (!epidb::utils::string_to_long(position, value)) {
          EPIDB_LOG_ERR("Invalid position '" + position + "' in the normalize_epigenetic_mark");
          return normalize_name(epigenetic_mark);
        }
        std::string norm_position = utils::integer_to_string(value);
        std::string norm_histone_modification = std::string("h") + histone + aa + norm_position + rest;
        return norm_histone_modification;
      }

      return l;
    }

    std::string format_extra_metadata(const mongo::BSONObj &key_value)
    {
      StringBuilder sb;
      auto it = key_value.begin();

      while (it.more() ) {
        auto const &e = it.next();
        std::string field_name = std::string(e.fieldName());

        if (field_name.compare(0, 2, "__") == 0) {
          continue;
        }

        if (field_name.compare(0, 5, "norm_") == 0) {
          continue;
        }

        std::string content = std::string(bson_to_string(e));
        sb.append("<b>");
        sb.append(field_name);
        sb.append("</b>: ");
        sb.append(content);
        sb.append("<br/>");
      }

      return sb.to_string();
    }

    // FROM https://github.com/mongodb/mongo-cxx-driver/blob/6dc65e99af9979152deb0940b2313c560e61e2d9/src/mongo/bson/bsonelement.cpp
    std::string bson_to_string(const mongo::BSONElement &e)
    {
      switch ( e.type() ) {
      case mongo::Date: {
        std::time_t date = e.Date().toTimeT();
        std::tm *ptm = std::localtime(&date);
        char buffer[32];
        // Format: Mo, 15.06.2009 20:20:00
        std::strftime(buffer, 32, "%d.%m.%Y %H:%M:%S", ptm);
        return std::string(buffer);
      }
      case mongo::NumberDouble: {
        return score_to_string(e.Double());
      }
      case mongo::NumberLong: {
        return long_to_string(e.Long());
      }
      case mongo::NumberInt: {
        return integer_to_string(e.Int());
      }
      case mongo::Bool: {
        return ( e.boolean() ? "true" : "false" );
      }
      case mongo::Symbol:
      case mongo::String: {
        return e.valuestr();
      }
      case mongo::Object: {
        return e.Obj().toString();
      }
      case mongo::Array: {
        std::string s = "";
        std::vector<mongo::BSONElement>  ee = e.Array();
        bool first = true;
        for (auto element : ee) {
          if (!first) {
            s += ",";
          } else {
            first = false;
          }
          s += bson_to_string(element);
        }
        return s;
      }
      default: {
        return "[type " + integer_to_string(e.type()) + " not implemented to string conversion]";
      }
      }
    }

    serialize::ParameterPtr element_to_parameter(const mongo::BSONElement& e)
    {
      switch ( e.type() ) {
      case mongo::NumberDouble: {
        return std::make_shared<serialize::SimpleParameter>(e.Double());
      }
      case mongo::NumberInt:
      case mongo::NumberLong: {
        return std::make_shared<serialize::SimpleParameter>((long long) e.numberLong());
      }
      case mongo::Object: {
        return bson_to_parameters(e.Obj());
      }
      case mongo::Array: {
        serialize::ParameterPtr p = std::make_shared<serialize::ListParameter>();
        std::vector<mongo::BSONElement> ees = e.Array();
        for (auto const& ee : ees) {
          p->add_child(element_to_parameter(ee));
        }
        return p;
      }
      case mongo::Bool: {
        return std::make_shared<serialize::SimpleParameter>(e.boolean());
      }
      case mongo::Date: {
        return std::make_shared<serialize::SimpleParameter>((long long)e.date());
      }
      default: {
        return std::make_shared<serialize::SimpleParameter>(e.String());
      }
      }
    }

    serialize::ParameterPtr bson_to_parameters(const mongo::BSONObj & o)
    {
      serialize::ParameterPtr parameter(new serialize::MapParameter());

      for (mongo::BSONObj::iterator it = o.begin(); it.more(); ) {
        mongo::BSONElement e = it.next();

        std::string fieldname = e.fieldName();

        if (!fieldname.compare(0, 2, "__")) {
          continue;
        }

        parameter->add_child(fieldname, element_to_parameter(e));
      }

      return parameter;
    }


    std::string sanitize(const std::string &data)
    {
      std::string buffer;
      buffer.reserve(data.size() * 1.05);
      for (size_t pos = 0; pos != data.size(); ++pos) {
        switch (data[pos]) {
        case '&':  buffer.append("&amp;");       break;
        case '\"': buffer.append("&quot;");      break;
        case '\'': buffer.append("&apos;");      break;
        case '<':  buffer.append("&lt;");        break;
        case '>':  buffer.append("&gt;");        break;
        default:   buffer.append(&data[pos], 1); break;
        }
      }
      return buffer;
    }

    bool is_id(const std::string &id, const std::string &prefix)
    {

      if (id.size() <= prefix.size()) {
        return false;
      }

      if (id.compare(0, prefix.size(), prefix) != 0) {
        return false;
      }

      return is_number(id.substr(prefix.size()));
    }

// TODO: Use template
    mongo::BSONArray build_array(const std::vector<long> &params)
    {
      mongo::BSONArrayBuilder ab;
      for (const auto& param : params) {
        ab.append((long long)param);
      }
      return ab.arr();
    }

    // TODO: move to arrays.cpp
    // TODO: Use template
    mongo::BSONArray build_array(const std::vector<std::string> &params)
    {
      mongo::BSONArrayBuilder ab;
      for (const auto& param : params) {
        ab.append(param);
      }
      return ab.arr();
    }

    mongo::BSONArray build_regex_array(const std::vector<std::string> &params)
    {
      mongo::BSONArrayBuilder ab;
      for (const auto& param : params) {
        ab.appendRegex(param, "i");
      }
      return ab.arr();
    }

    mongo::BSONArray build_normalized_array(const std::vector<std::string> &params)
    {
      mongo::BSONArrayBuilder ab;
      for (const auto& param : params) {
        ab.append(utils::normalize_name(param));
      }
      return ab.arr();
    }

    mongo::BSONArray build_array_long(const std::vector<serialize::ParameterPtr> &params)
    {
      mongo::BSONArrayBuilder ab;
      for (const auto& param : params) {
        ab.append(param->as_long());
      }
      return ab.arr();
    }

    mongo::BSONArray build_array_long(const std::vector<long> &params)
    {
      mongo::BSONArrayBuilder ab;
      for (const auto& param : params) {
        ab.append((long long)param);
      }
      return ab.arr();
    }

    mongo::BSONArray build_array(const std::vector<serialize::ParameterPtr> &params)
    {
      mongo::BSONArrayBuilder ab;
      for (const auto& param : params) {
        ab.append(param->as_string());
      }
      return ab.arr();
    }

    mongo::BSONArray build_normalized_array(const std::vector<serialize::ParameterPtr> &params)
    {
      mongo::BSONArrayBuilder ab;
      for (const auto& param : params) {
        ab.append(utils::normalize_name(param->as_string()));
      }
      return ab.arr();
    }

    mongo::BSONArray build_epigenetic_normalized_array(const std::vector<serialize::ParameterPtr> &params)
    {
      mongo::BSONArrayBuilder ab;
      for (const auto& param : params) {
        ab.append(utils::normalize_epigenetic_mark(param->as_string()));
      }
      return ab.arr();
    }

    mongo::BSONArray build_annotation_normalized_array(const std::vector<serialize::ParameterPtr> &params)
    {
      mongo::BSONArrayBuilder ab;
      for (const auto& param : params) {
        ab.append(utils::normalize_annotation_name(param->as_string()));
      }
      return ab.arr();
    }

    std::vector<std::string> build_vector(const std::vector<serialize::ParameterPtr> &params)
    {
      std::vector<std::string> vector;
      for (const auto& param : params) {
        vector.push_back(param->as_string());
      }
      return vector;
    }

    std::vector<std::string> build_vector(const std::vector<mongo::BSONElement> &params)
    {
      std::vector<std::string> vector;
      for (auto be : params) {
        vector.push_back(be.str());
      }
      return vector;
    }

    std::vector<long> build_vector_long(const std::vector<mongo::BSONElement> &params)
    {
      std::vector<long> vector;
      for (auto be : params) {
        vector.push_back(be.Long());
      }
      return vector;
    }

    std::set<std::string> build_set(const std::vector<mongo::BSONElement> &params)
    {
      std::set<std::string> set;
      for (auto be : params) {
        set.insert(be.str());
      }
      return set;
    }

    bool check_parameters(const std::vector<serialize::ParameterPtr> &params, const std::function<std::string(const std::string&)> &normalizer, const std::function<bool(const std::string&)> &checker, std::string &wrong)
    {
      std::vector<std::string> names = build_vector(params);
      return check_parameters(names, normalizer, checker, wrong);
    }

    bool check_parameters(const std::vector<std::string> &names, const std::function<std::string(const std::string&)> &normalizer, const std::function<bool(const std::string&)> &checker, std::string &wrong)
    {
      for (auto  name : names) {
        std::string norm_name = normalizer(name);
        if (!checker(norm_name)) {
          wrong = name;
          return false;
        }
      }
      return true;
    }


  }
}
