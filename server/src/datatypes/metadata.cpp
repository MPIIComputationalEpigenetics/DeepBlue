//
//  metadata.cpp
//  epidb
//
//  Created by Felipe Albrecht on 07.10.14.
//  Copyright (c) 2016 Max Planck Institute for Informatics. All rights reserved.
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

      mongo::BSONObj::iterator i = obj.begin();
      while (i.more() ) {
        const mongo::BSONElement &e = i.next();
        extra_metadata[e.fieldName()] = utils::bson_to_string(e);
      }

      return extra_metadata;
    }
  }
}