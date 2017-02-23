//
//  connected_cache.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 22.02.17.
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


#ifndef EPIDB_CONNECTED_CACHE_HPP
#define EPIDB_CONNECTED_CACHE_HPP

#include <map>
#include <string>

namespace epidb {
  class ConnectedCache {
  private:
    typedef std::map<std::string, bool> ConnecterdCached;
    typedef std::map<std::string, ConnecterdCached> CacheMapList;

    CacheMapList __cache_is_connected;

    void add_connection(const std::string &bs1, const std::string &bs2)
    {
      CacheMapList::iterator it = __cache_is_connected.find(bs1);
      if (it != __cache_is_connected.end()) {
        ConnecterdCached &terms = it->second;
        terms[bs2] = true;
      } else {
        ConnecterdCached terms;
        terms[bs2] = true;
        __cache_is_connected[bs2] = terms;
      }
    }

    void add_pair(const std::string &bs1, const std::string &bs2)
    {
      add_connection(bs1, bs2);
      add_connection(bs2, bs1);
    }

  public:
    void set_connection(const std::string &bs1, const std::string &bs2)
    {
      add_pair(bs1, bs2);
    }

    bool is_connected(const std::string &bs1, const std::string &bs2)
    {
      CacheMapList::iterator it = __cache_is_connected.find(bs1);
      if (it != __cache_is_connected.end()) {
        return it->second.find(bs2) != it->second.end();
      }
      return false;
    }

    void invalidate()
    {
      __cache_is_connected.clear();
    }
  };
}

#endif