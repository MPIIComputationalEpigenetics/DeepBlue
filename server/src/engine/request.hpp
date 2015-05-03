//
//  request.cpp
//  epidb
//
//  Created by Felipe Albrecht on 27.01.15.
//  Copyright (c) 2013,2014,2015 Max Planck Institute for Computer Science. All rights reserved.
//

#ifndef REQUEST_STATUS_HPP
#define REQUEST_STATUS_HPP

#include <vector>

#include "../extras/utils.hpp"

namespace epidb {
  namespace request {

    using StringDataPair = std::pair<std::string, std::string>;
    using IntegerDataPair = std::pair<std::string, long long>;
    using FloatDataPair = std::pair<std::string, float>;

    typedef enum {
      INVALID,
      REGIONS,
      ID_NAMES,
      MAP
    } DataType;

    typedef struct Status {
      std::string state;
      std::string message;
    } Status;
    
    typedef struct Job {
      std::string _id;
      Status status;
    } Job;
    
    typedef struct Data {
      std::vector<utils::IdName> id_names;
      std::vector<StringDataPair> strings;
      std::vector<IntegerDataPair> integers;
      std::vector<FloatDataPair> floats;

      void set_id_names(std::vector<utils::IdName> &&_id_names)
      {
        id_names = _id_names;
      }

      void append(const std::string &key, const std::string &value)
      {
        strings.push_back(StringDataPair(key, value));
      }

      void append(const std::string &key, const long long value)
      {
        integers.push_back(IntegerDataPair(key, value));
      }

      void append(const std::string &key, const float value)
      {
        floats.push_back(IntegerDataPair(key, value));
      }
    public:
      void load_from_bson(mongo::BSONObj &o)
      {
        for (mongo::BSONObj::iterator it = o.begin(); it.more(); ) {
          mongo::BSONElement e = it.next();
          std::cerr << e.toString() << std::endl;

          std::string fieldname = e.fieldName();

          if (!fieldname.compare(0, 2, "__")) {
            continue;
          }

          switch ( e.type() ) {

          case mongo::NumberDouble: {
            append(fieldname, (float) e.Double());
            break;
          }
          case mongo::NumberLong: {
            append(fieldname, (long long) e.Long());
            break;
          }
          case mongo::NumberInt: {
            append(fieldname, (long long) e.Int());
            break;
          }

          default: {
            append(fieldname, utils::bson_to_string(e));
          }

          }
        }
      }
    } Data;
  }
}

#endif