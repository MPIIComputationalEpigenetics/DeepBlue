//
//  request.cpp
//  epidb
//
//  Created by Felipe Albrecht on 27.01.15.
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

#ifndef REQUEST_STATUS_HPP
#define REQUEST_STATUS_HPP

#include <vector>

#include "../datatypes/metadata.hpp"
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
      boost::posix_time::ptime create_time;
      std::string command;
      std::string user_id;
      datatypes::Metadata misc;
      /*
      * \brief Time at which the job was finished. Only to be set, if status == TS_DONE
      */
      boost::posix_time::ptime finish_time;
      std::string query_id;
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