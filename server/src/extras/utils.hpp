//
//  utils.hpp
//  epidb
//
//  Created by Felipe Albrecht on 27.06.13.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#ifndef EPIDB_UTILS_HPP
#define EPIDB_UTILS_HPP

#include <vector>
#include <string>
#include <algorithm>
#include <sstream>
#include <iterator>
#include <iostream>

#include <mongo/bson/bson.h>

#include "serialize.hpp"

#include "../types.hpp"

namespace epidb {
  namespace utils {

    class IdName {
    public:
      IdName() :
        id(""), name("") {}
      IdName(std::string i, std::string n) :
        id(i), name(n) {}
      std::string id;
      std::string name;
    };

    std::ostream &operator<<(std::ostream &os, const IdName &o);

    class IdNameCount {
    public:
      IdNameCount() :
        id(""), name(""), count(0) {}
      IdNameCount(std::string i, std::string n, size_t c) :
        id(i), name(n), count(c) {}
      std::string id;
      std::string name;
      size_t count;
    };

    std::ostream &operator<<(std::ostream &os, const IdName &o);

    std::ostream &operator<<(std::ostream &os, const IdNameCount &o);

    template <typename Type> std::string vector_to_string(std::vector<Type> vector, std::string sep=",")
    {
      std::ostringstream oss;
      if (!vector.empty()) {
        std::ostream_iterator<Type> oss_it(oss, sep.c_str());
        std::copy(vector.begin(), vector.end() - 1, oss_it);
        oss << vector.back();
      }
      return oss.str();
    }

    class ExperimentInfo {
    public:
      ExperimentInfo(std::string &i, std::string &n, std::string &em, std::string s, std::string &t,
                     std::string &p, std::string &d):
        id(i),
        name(n),
        epigenetic_mark(em),
        sample_id(s),
        technique(t),
        project(p),
        description(d) {}

      std::string id;
      std::string name;
      std::string epigenetic_mark;
      std::string sample_id;
      std::string technique;
      std::string project;
      std::string description;
    };


    std::vector<std::string> string_to_vector(const std::string &s, const char sep = ',');

    std::pair<std::string, std::string> string_to_pair(const std::string &s, const char sep = ',');

    bool string_to_int(const std::string &s, int &i);

    bool string_to_long(const std::string &s, size_t &i);

    bool string_to_double(const std::string &s, double &d);

    bool string_to_float(const std::string &s, float &d);

    bool string_to_position(const std::string &s, Position &p);

    bool string_to_length(const std::string &s, Length p);

    bool string_to_score(const std::string &s, Score &c);

    const std::string double_to_string(const double d);

    const std::string integer_to_string(const int d);

    bool is_number(const std::string &s);

    bool valid_input_string(const std::string &in);

    const std::string lower(const std::string &in);

    const std::string upper(const std::string &in);

    const std::string normalize_name(const std::string &name);

    const std::string normalize_annotation_name(const std::string &annotation_name);

    const std::string normalize_epigenetic_mark(const std::string &histone_modification);

    const std::string bson_to_string(const mongo::BSONElement &e);
  }
}

#endif /* defined(EPIDB_UTILS_HPP) */
