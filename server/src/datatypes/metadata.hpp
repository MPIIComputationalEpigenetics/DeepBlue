//
//  metadata.hpp
//  epidb
//
//  Created by Felipe Albrecht on 07.10.14.
//  Copyright (c) 2016 Max Planck Institute for Informatics. All rights reserved.
//

#ifndef METADATA_HPP
#define METADATA_HPP

#include <map>
#include <string>

namespace mongo {
  class BSONObj;
}

namespace epidb {
  namespace datatypes {
    typedef std::map<std::string, std::string> Metadata;

    mongo::BSONObj metadata_to_bson(const Metadata &extra_metadata);
    Metadata bson_to_metadata(const mongo::BSONObj& obj);
  }
}

#endif
