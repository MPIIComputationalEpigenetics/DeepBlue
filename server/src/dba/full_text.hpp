//
//  full_text.hpp
//  epidb
//
//  Created by Felipe Albrecht on 30.04.14.
//  Copyright (c) 2014 Max Planck Institute for Computer Science. All rights reserved.
//

#ifndef EPIDB_DBA_FULL_TEXT_HPP
#define EPIDB_DBA_FULL_TEXT_HPP


#include <map>
#include <string>
#include <sstream>
#include <vector>

#include <time.h>
#include <math.h>

#include <boost/foreach.hpp>

#include <mongo/bson/bson.h>

namespace epidb {
  namespace dba {
    // TODO: rename namespace to full_text
    namespace search {

      struct TextSearchResult {
        std::string id;
        std::string name;
        std::string type;

        double score;
      };

      // TODO: rename namespace to insert
      bool insert_full_text(const std::string &type, const std::string &id,
                            const mongo::BSONObj &extra_data, std::string &msg);

      // TODO: rename namespace to search
      bool search_full_text(const std::string &text, std::vector<TextSearchResult> &, std::string &msg);

      bool insert_related_term(const std::string &id, const std::string &name, std::string &msg);

      bool change_extra_metadata_full_text(const std::string &id, const std::string &key, const std::string &value, std::string &msg);

      // TODO: rename namespace to search
      bool search_full_text(const std::string &text, const std::vector<std::string> &types,
                            std::vector<TextSearchResult> &, std::string &msg);

      bool remove(const std::string &id, std::string &msg);
    }
  }
}

#endif
