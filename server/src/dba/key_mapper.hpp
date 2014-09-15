//
//  key_mapper.hpp
//  epidb
//
//  Created by Fabian Reinartz on 01.07.13.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#ifndef EPIDB_DBA_KEYMAPPER_HPP
#define EPIDB_DBA_KEYMAPPER_HPP

#include <map>
#include <string>

#include <boost/thread/mutex.hpp>

namespace epidb {
  namespace dba {

    class KeyMapper {
    public:
      static bool to_short(const std::string &s, std::string &res, std::string &err);
      static bool to_long(const std::string &s, std::string &res, std::string &err);
      static std::string build_default(const std::string &s);

      static const std::string& BED_COMPRESSED();
      static const std::string& BED_DATA();
      static const std::string& BED_DATASIZE();
      static const std::string& CHROMOSOME();
      static const std::string& DATASET();
      static const std::string& END();
      static const std::string& FEATURES();
      static const std::string& START();
      static const std::string& VALUE();
      static const std::string& WIG_TYPE();
      static const std::string& WIG_STEP();
      static const std::string& WIG_SPAN();
      static const std::string& WIG_DATA_SIZE();
      static const std::string& WIG_COMPRESSED();
      static const std::string& WIG_TRACK_TYPE();
      static const std::string& WIG_DATA();

    private:
      static bool set_shortcut(const std::string &s, const std::string &l, std::string &err);
      static bool read_database();

      static std::map<std::string, std::string> stol_;
      static std::map<std::string, std::string> ltos_;

      static bool loaded_;
      static boost::mutex read_database_mutex;
      static boost::mutex set_shortcut_mutex;
    };

  } // namespace dba
} // namespace epidb

#endif
