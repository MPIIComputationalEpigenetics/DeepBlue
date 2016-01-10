//
//  controlled_vocabulary.h
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 01.08.13.
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

#ifndef EPIDB_DBA_CONTROLED_VOCABULARY
#define EPIDB_DBA_CONTROLED_VOCABULARY

#include <iostream>
#include <vector>

#include "../extras/utils.hpp"

namespace epidb {
  namespace dba {
    namespace cv {

      class BiosourceConnectedCache {
      private:
        typedef std::map<std::string, bool> BiosourcesCached;
        typedef std::map<std::string, BiosourcesCached> CacheMapList;

        CacheMapList __cache_is_connected;

        void add_connection(const std::string &bs1, const std::string &bs2)
        {
          CacheMapList::iterator it = __cache_is_connected.find(bs1);
          if (it != __cache_is_connected.end()) {
            BiosourcesCached &biosources = it->second;
            biosources[bs2] = true;
          } else {
            BiosourcesCached biosources;
            biosources[bs2] = true;
            __cache_is_connected[bs2] = biosources;
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


      extern BiosourceConnectedCache biosources_cache;

      bool set_biosource_synonym_complete(const std::string &biosource_name, const std::string &synonym_name,
                                          const std::string& user_key, std::string& msg);

      bool get_biosource_synonyms(const std::string &id, const std::string &biosource_name, const std::string &norm_biosource_name,
                                  bool is_biosource, const std::string &user_key,
                                  std::vector<utils::IdName> &syns, std::string &msg);

      bool set_biosource_parent(const std::string &biosource_more_embracing, const std::string &norm_biosource_more_embracing,
                                const std::string &biosource_less_embracing, const std::string &norm_biosource_less_embracing,
                                bool more_embracing_is_syn, const bool less_embracing_is_syn,
                                const std::string &user_key, std::string &msg);

      bool get_biosource_parents(const std::string &biosource_name, const std::string &norm_biosource_name,
                                 bool is_biosource,
                                 const std::string &user_key,
                                 std::vector<std::string> &norm_uppers, std::string &msg);

      bool get_biosource_children(const std::string &biosource_name, const std::string &norm_biosource_name,
                                  bool is_biosource,
                                  const std::string &user_key,
                                  std::vector<std::string> &norm_subs, std::string &msg);

      bool get_synonym_root(const std::string &synonym, const std::string &norm_synonym,
                            std::string &biosource_name, std::string &norm_biosource_name, std::string &msg);

      bool remove_biosouce(const std::string &id, const std::string &biosource_name , const std::string &norm_biosource_name, std::string &msg);
    }
  }
}

#endif /* defined(EPIDB_DBA_CONTROLED_VOCABULARY) */