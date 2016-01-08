//
//  key_mapper.hpp
//  epidb
//
//  Created by Fabian Reinartz on 01.07.13.
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
      static const std::string& DATASET();
      static const std::string& END();
      static const std::string& FEATURES();
      static const std::string& START();
      static const std::string& VALUE();
      static const std::string& WIG_STEP();
      static const std::string& WIG_SPAN();
      static const std::string& WIG_DATA_SIZE();
      static const std::string& WIG_COMPRESSED();
      static const std::string& WIG_TRACK_TYPE();
      static const std::string& WIG_DATA();

      static const std::string& SEQNAME();
      static const std::string& SOURCE();
      static const std::string& CHROMOSOME();
      static const std::string& FEATURE();
      static const std::string& SCORE();
      static const std::string& STRAND();
      static const std::string& FRAME();
      static const std::string& ATTRIBUTES();
      static const std::string& GENE_ID();

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
