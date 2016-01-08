//
//  version.hpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 13.06.13.
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

#ifndef EPIDB_VERSION_HPP
#define EPIDB_VERSION_HPP

#include <sstream>
#include <string>

#include <boost/config.hpp>
#include <boost/version.hpp>

#include "extras/utils.hpp"

namespace epidb {
  class Version {
  public:
    static const std::string name;
    static const std::string copyright;
    static const std::string license;
    static const std::string terms;
    static const std::string author;
    static const std::string coauthors;
    static const size_t major_version;
    static const size_t minor_version;
    static const size_t fix_version;

    static int version_value() {
      return (major_version * 100 * 100) + (minor_version * 100) + (fix_version);
    }

    static const std::string version() {
      static std::string _version_cache;

      if (_version_cache.length() == 0) {
        _version_cache = utils::integer_to_string(major_version) + std::string(".") +
          utils::integer_to_string(minor_version) + std::string(".") +
          utils::integer_to_string(fix_version);
      }

      return _version_cache;
    }

    static const std::string info() {
      std::ostringstream oss;
      oss << name << " - " << version() << std::endl;
      oss << copyright << std::endl;
      oss << "by " << author << " with " << coauthors << std::endl;
      oss << " compiled with " << BOOST_PLATFORM << " " << BOOST_COMPILER << " at " << __DATE__ ", " __TIME__ "." << std::endl;
      oss << " using standard library: " << BOOST_STDLIB;
      oss << " and BOOST: " << (BOOST_VERSION/100000) << "." << ((BOOST_VERSION / 100) % 1000) << "." << (BOOST_VERSION % 100) << std::endl;
      return oss.str();
    }

  };
}

#endif
