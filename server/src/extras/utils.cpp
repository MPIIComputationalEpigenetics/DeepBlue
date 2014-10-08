//
//  utils.cpp
//  epidb
//
//  Created by Felipe Albrecht on 02.07.13.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
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

    std::vector<std::string> string_to_vector(const std::string &s, const char sep)
    {
      std::vector<std::string> tokens;
      std::istringstream ss(s);
      std::string token;

      while (std::getline(ss, token, sep)) {
        tokens.push_back(token);
      }

      return tokens;
    }

    std::pair<std::string, std::string> string_to_pair(const std::string &s, const char sep)
    {
      std::string ss(s);
      boost::trim(ss);
      std::size_t found = s.find_first_of(sep);
      if (found == std::string::npos) {
        return std::pair<std::string, std::string>(s, "");
      }
      return std::pair<std::string, std::string>(s.substr(0, found), s.substr(found + 1));
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
      if (neg)  {
        ++p;
      }
      while (*p) {
        if (!std::isdigit(*p)) {
          return false;
        }
        num += decdigits[(*p++ - '0') * 10 + pos--];
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

    bool string_to_length(const std::string &s, Length &l)
    {
      return string_to_fixed_point<Length>(s, l);
    }

    template<typename T>
    bool string_to_floating_point(T &r, const char *p)
    {
      // Skip leading white space, if any.
      while (std::isspace(*p) ) {
        p += 1;
      }

      r = 0.0;
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

    bool string_to_double(const std::string &s_, double &d)
    {
      return string_to_floating_point<double>(d, s_.c_str());
    }

    bool string_to_float(const std::string &s_, float &d)
    {
      return string_to_floating_point<float>(d, s_.c_str());
    }

    bool string_to_score(const std::string &s_, Score &c)
    {
      return string_to_floating_point<Score>(c, s_.c_str());
    }

    const std::string double_to_string(const double d)
    {
      return fmt::format("{:-.4f}", d);
    }

    const std::string integer_to_string(const int t)
    {
      return fmt::FormatInt(t).str();
    }

    const std::string long_to_string(const long t)
    {
      return fmt::FormatInt(t).str();
    }

    bool is_number(const std::string &s_)
    {
      std::string ss(s_);
      boost::trim(ss);
      std::string::const_iterator it = ss.begin();
      while (it != ss.end() && std::isdigit(*it)) ++it;
      return !ss.empty() && it == ss.end();
    }

    bool valid_input_string(const std::string &in)
    {
      std::string out;
      for (std::string::const_iterator it = in.begin(); it != in.end(); ++it) {
        char c = *it;

        if (c == '[' ||
            c == ']' ||
            c == '{' ||
            c == '}' ||
            c == '$')

          return false;
      }

      return true;
    }

    const std::string lower(const std::string &in)
    {
      std::string data(in);
      std::transform(data.begin(), data.end(), data.begin(), ::tolower);
      return data;
    }

    const std::string upper(const std::string &in)
    {
      std::string data(in);
      std::transform(data.begin(), data.end(), data.begin(), ::toupper);
      return data;
    }

    const std::string normalize_name(const std::string &in)
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

    const std::string normalize_annotation_name(const std::string &annotation_name)
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

    const std::string normalize_epigenetic_mark(const std::string &epigenetic_mark)
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

    // FROM https://github.com/mongodb/mongo-cxx-driver/blob/6dc65e99af9979152deb0940b2313c560e61e2d9/src/mongo/bson/bsonelement.cpp
    const std::string bson_to_string(const mongo::BSONElement &e)
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
        return double_to_string(e.Double());
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
      default: {
        return "[type " + integer_to_string(e.type()) + " not implemented to string conversion]";
      }
      }
    }
  }
}
