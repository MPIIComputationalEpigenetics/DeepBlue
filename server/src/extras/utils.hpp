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

#include "serialize.hpp"

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

    template <typename Type> std::string vector_to_string(std::vector<Type> vector)
    {
      std::ostringstream oss;
      if (!vector.empty()) {
        std::ostream_iterator<Type> oss_it(oss, ",");
        std::copy(vector.begin(), vector.end() - 1, oss_it);
        oss << vector.back();
      }
      return oss.str();
    }

    std::vector<std::string> string_to_vector(const std::string &s, const char sep = ',');

    std::pair<std::string, std::string> string_to_pair(const std::string &s, const char sep = ',');

    bool string_to_long(const std::string &s, size_t &i);

    bool string_to_double(const std::string &s, double &d);

    const std::string double_to_string(const double d);

    bool is_number(const std::string &s);

    bool valid_input_string(const std::string &in);

    const std::string lower(const std::string &in);

    const std::string upper(const std::string &in);

    const std::string normalize_name(const std::string &name);

    const std::string normalize_annotation_name(const std::string &annotation_name);

    const std::string normalize_epigenetic_mark(const std::string &histone_modification);
  }
}

#endif /* defined(EPIDB_UTILS_HPP) */
