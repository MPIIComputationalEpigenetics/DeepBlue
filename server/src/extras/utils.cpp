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
#include <ctype.h>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>

#include "../log.hpp"

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

    // XXX: Template for string_to_long and string_to_double
    bool string_to_long(const std::string &s_, size_t &i)
    {
      std::string ss(s_);
      boost::trim(ss);
      try {
        i = boost::lexical_cast<size_t>(ss);
        return true;
      } catch (boost::bad_lexical_cast const &) {
        i = LONG_MIN;
        return false;
      }
    }

    bool string_to_double(const std::string &s_, double &d)
    {
      std::string ss(s_);
      boost::trim(ss);
      try {
        d = boost::lexical_cast<double>(ss);
        return true;
      } catch (boost::bad_lexical_cast const &m) {
        d = NAN;
        return false;
      }
    }

    const std::string double_to_string(const double d)
    {
      std::ostringstream sstream;
      sstream << std::fixed;
      sstream << std::setprecision(4) << d;
      return sstream.str();
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
        std::string norm_position = boost::lexical_cast<std::string>(value);
        std::string norm_histone_modification = std::string("h") + histone + aa + norm_position + rest;
        return norm_histone_modification;
      }

      return l;
    }
  }
}

