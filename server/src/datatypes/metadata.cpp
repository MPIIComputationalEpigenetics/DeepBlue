//
//  metadata.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 07.10.14.
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

#include <mongo/bson/bson.h>

#include "../extras/utils.hpp"

#include "metadata.hpp"

namespace epidb {
  namespace datatypes {
    mongo::BSONObj metadata_to_bson(const Metadata &extra_metadata)
    {
      mongo::BSONObjBuilder extra_metadata_builder;
      Metadata::const_iterator cit;
      for (cit = extra_metadata.begin(); cit != extra_metadata.end(); ++cit) {
        extra_metadata_builder.append(cit->first, cit->second);
      }
      return extra_metadata_builder.obj();
    }


    Metadata bson_to_metadata(const mongo::BSONObj& obj)
    {
      Metadata extra_metadata;

      auto i = obj.begin();
      while (i.more() ) {
        const mongo::BSONElement &e = i.next();
        extra_metadata[e.fieldName()] = utils::bson_to_string(e);
      }

      return extra_metadata;
    }
  }
}