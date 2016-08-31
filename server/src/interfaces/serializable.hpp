//
//  serializable.hpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 10.08.16.
//  Copyright (c) 2015 Max Planck Institute for Computer Science. All rights
//  reserved.
//
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

#ifndef SERIALIZABLE_HPP
#define SERIALIZABLE_HPP

#include <memory>
#include <vector>

namespace mongo {
  class BSONObj;
}

namespace epidb {
  class ISerializable {
  public:
    virtual const mongo::BSONObj to_BSON() = 0;
  };
  typedef std::unique_ptr<ISerializable> ISerializablePtr;


  // A file content
  class ISerializableFile {
  protected:
    std::vector<ISerializablePtr> _data;

  public:
    const std::vector<ISerializablePtr>& rows() const
    {
      return _data;
    }

    size_t size() const
    {
      return _data.size();
    }
  };

  typedef std::shared_ptr<ISerializableFile> ISerializableFilePtr;
}


#endif