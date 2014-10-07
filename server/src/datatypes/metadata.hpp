//
//  metadata.hpp
//  epidb
//
//  Created by Felipe Albrecht on 07.10.14.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include <map>

#include <mongo/bson/bson.h>

namespace epidb {
  namespace datatypes {
    typedef std::map<std::string, std::string> Metadata;

    mongo::BSONObj extra_metadata_to_bson(const Metadata &extra_metadata);
  }
}