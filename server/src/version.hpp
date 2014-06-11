//
//  version.hpp
//  epidb
//
//  Created by Felipe Albrecht on 13.06.13.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#ifndef EPIDB_VERSION_HPP
#define EPIDB_VERSION_HPP

#include <sstream>
#include <string>

#include <boost/config.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/version.hpp>

namespace epidb {
  class Version {
  public:
    static const std::string name;
    static const std::string copyright;
    static const std::string author;
    static const std::string coauthors;
    static const size_t major_version;
    static const size_t minor_version;
    static const size_t fix_version;

    static const std::string version() {
      static std::string _version_cache;

      if (_version_cache.length() == 0) {
        _version_cache = boost::lexical_cast<std::string>(major_version) + std::string(".") +
          boost::lexical_cast<std::string>(minor_version) + std::string(".") +
          boost::lexical_cast<std::string>(fix_version);
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
