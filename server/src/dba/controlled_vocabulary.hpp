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

namespace epidb {
  namespace dba {
    namespace cv {

      extern std::map<std::string, std::string> cache_is_connected;

      bool set_bio_source_synonym(const std::string &actual_existing_bio_source_name, const std::string &new_bio_source_name,
                                  bool is_bio_source, const bool is_syn,
                                  const std::string &user_key, std::string &msg);

      bool get_bio_source_synonyms(const std::string &bio_source_name, const std::string &norm_bio_source_name,
                                   bool is_bio_source, const std::string &user_key,
                                   std::vector<utils::IdName> &syns, std::string &msg);

      bool set_bio_source_embracing(const std::string &bio_source_more_embracing, const std::string &norm_bio_source_more_embracing,
                                    const std::string &bio_source_less_embracing, const std::string &norm_bio_source_less_embracing,
                                    bool more_embracing_is_syn, const bool less_embracing_is_syn,
                                    const std::string &user_key, std::string &msg);

      bool get_bio_source_scope(const std::string &bio_source_name, const std::string &norm_bio_source_name,
                                bool is_bio_source, const std::string &user_key,
                                std::vector<utils::IdName> &related_bio_sources,
                                std::string &msg);

      bool get_upper_term(const std::string &bio_source_name, const std::string &norm_bio_source_name,
                          bool is_bio_source,
                          std::string &upper,
                          const std::string &user_key, std::string &msg);

      bool get_bio_source_embracing(const std::string &bio_source_name, const std::string &norm_bio_source_name,
                                    bool is_bio_source, const std::string &user_key,
                                    std::vector<std::string> &norm_subs, std::string &msg);

      bool get_synonym_root(const std::string &synonym, const std::string &norm_synonym,
                            std::string &bio_source_name, std::string &norm_bio_source_name, std::string &msg);
    }
  }
}

#endif /* defined(EPIDB_DBA_CONTROLED_VOCABULARY) */