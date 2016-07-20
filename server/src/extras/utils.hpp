//
//  utils.hpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 27.06.13.
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

#ifndef EPIDB_UTILS_HPP
#define EPIDB_UTILS_HPP

#include <algorithm>
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

    std::vector<IdName> bsons_to_id_names(const std::vector<mongo::BSONObj> &bsons);
    IdName bson_to_id_name(const mongo::BSONObj& bson);


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

    std::vector<std::string> capitalize_vector(std::vector<std::string> vector);

    std::string lower_case(std::string string);

    template <typename Type> std::string vector_to_string(std::vector<Type> vector, std::string sep = ",")
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


    bool string_to_int(const std::string &s, int &i);

    bool string_to_long(const std::string &s, size_t &i);

    bool string_to_position(const std::string &s, Position &p);

    bool string_to_score(const std::string &s, Score &c);

    std::string score_to_string(const Score s);

    std::string integer_to_string(const int d);

    std::string size_t_to_string(const size_t t);

    std::string long_to_string(const long t);

    bool is_number(const std::string &s);

    std::string lower(const std::string &in);

    std::string normalize_name(const std::string &name);

    std::string normalize_annotation_name(const std::string &annotation_name);

    std::string normalize_epigenetic_mark(const std::string &histone_modification);

    std::string format_extra_metadata(const mongo::BSONObj &key_value);

    std::string bson_to_string(const mongo::BSONElement &e);

    serialize::ParameterPtr bson_to_parameters(const mongo::BSONObj & o);

    serialize::ParameterPtr element_to_parameter(const mongo::BSONElement& e);

    std::string sanitize(const std::string &data);

    mongo::BSONArray build_array(const std::vector<std::string> &params);
    mongo::BSONArray build_array(const std::vector<long> &params);
    mongo::BSONArray build_array(const std::vector<serialize::ParameterPtr> &params);
    mongo::BSONArray build_array_long(const std::vector<serialize::ParameterPtr> &params);
    mongo::BSONArray build_array_long(const std::vector<long> &params);
    mongo::BSONArray build_regex_array(const std::vector<std::string> &params);
    mongo::BSONArray build_normalized_array(const std::vector<std::string> &params);
    mongo::BSONArray build_normalized_array(const std::vector<serialize::ParameterPtr> &params);
    mongo::BSONArray build_epigenetic_normalized_array(const std::vector<serialize::ParameterPtr> &params);
    mongo::BSONArray build_annotation_normalized_array(const std::vector<serialize::ParameterPtr> &params);

    std::vector<std::string> build_vector(const std::vector<serialize::ParameterPtr> &params);
    std::vector<std::string> build_vector(const std::vector<mongo::BSONElement> &params);
    std::vector<long> build_vector_long(const std::vector<mongo::BSONElement> &params);
    std::set<std::string> build_set(const std::vector<mongo::BSONElement> &params);

    bool check_parameters(const std::vector<serialize::ParameterPtr> &params, const std::function<std::string(const std::string&)> &normalizer, const std::function<bool(const std::string&)> &checker, std::string &wrong);

    bool check_parameters(const std::vector<std::string> &params, const std::function<std::string(const std::string&)> &normalizer, const std::function<bool(const std::string&)> &checker, std::string &wrong);

    bool is_id(const std::string &id, const std::string &prefix);
  }
}

#endif /* defined(EPIDB_UTILS_HPP) */
