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

namespace epidb {
  namespace request {

    using StringDataPair = std::pair<std::string, std::string>;
    using IntegerDataPair = std::pair<std::string, long long>;
    using FloatDataPair = std::pair<std::string, float>;

    typedef struct Status {
      std::string state;
      std::string message;
    } Status;

    typedef struct Data {
      std::vector<StringDataPair> strings;
      std::vector<IntegerDataPair> integers;
      std::vector<FloatDataPair> floats;
      std::unique_ptr<std::string> data;

      void append(const std::string key, const std::string value)
      {
        strings.push_back(StringDataPair(key, value));
      }

      void append(const std::string key, const long long value)
      {
        integers.push_back(IntegerDataPair(key, value));
      }

      void append(const std::string key, const float value)
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