//
//  metadata.hpp
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
