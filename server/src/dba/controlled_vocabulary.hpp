//
//  controlled_vocabulary.h
//  epidb
//
//  Created by Felipe Albrecht on 01.08.13.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#ifndef EPIDB_DBA_CONTROLED_VOCABULARY
#define EPIDB_DBA_CONTROLED_VOCABULARY

#include <iostream>
#include <vector>

#include "../extras/utils.hpp"

namespace epidb {
  namespace dba {
    namespace cv {

      extern std::map<std::string, std::string> cache_is_connected;

      bool set_biosource_synonym(const std::string &actual_existing_biosource_name, const std::string &new_biosource_name,
                                 bool is_biosource, const bool is_syn,
                                 const std::string &user_key, std::string &msg);

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