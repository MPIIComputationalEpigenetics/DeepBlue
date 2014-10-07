//
//  metadata.cpp
//  epidb
//
//  Created by Felipe Albrecht on 07.10.14.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include <mongo/bson/bson.h>

#include "metadata.hpp"

namespace epidb {
  namespace datatypes {
    mongo::BSONObj extra_metadata_to_bson(const Metadata &extra_metadata)
    {
      mongo::BSONObjBuilder extra_metadata_builder;
      Metadata::const_iterator cit;
      for (cit = extra_metadata.begin(); cit != extra_metadata.end(); ++cit) {
        extra_metadata_builder.append(cit->first, cit->second);
      }
      return extra_metadata_builder.obj();
    }
  }
}