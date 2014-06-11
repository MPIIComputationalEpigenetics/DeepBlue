//
//  list.hpp
//  epidb
//
//  Created by Felipe Albrecht on 07.04.14.
//  Copyright (c) 2014 Max Planck Institute for Computer Science. All rights reserved.
//

#ifndef EPIDB_DBA_LIST_HPP
#define EPIDB_DBA_LIST_HPP

#include <map>
#include <string>
#include <sstream>
#include <vector>

#include <mongo/bson/bson.h>
#include <mongo/client/dbclient.h>

#include "dba.hpp"

#include "../engine/commands.hpp"

#include "../extras/utils.hpp"

namespace epidb {
  namespace dba {
    namespace list {

      /**
       * Lists
       */

      bool genomes(const std::string &user_key, std::vector<utils::IdName> &result, std::string &msg);

      bool bio_sources(const std::string &user_key, std::vector<utils::IdName> &result, std::string &msg);

      bool samples(const std::string &user_key, const mongo::BSONArray &bio_sources, const Metadata &metadata,
                   std::vector<std::string> &result, std::string &msg);

      bool projects(const std::string &user_key, std::vector<utils::IdName> &result, std::string &msg);

      bool epigenetic_marks(const std::string &user_key, std::vector<utils::IdName> &result, std::string &msg);

      bool experiments(const mongo::Query query, std::vector<utils::IdName> &result, std::string &msg);

      bool annotations(const std::string &genome, const std::string &user_key,
                       std::vector<utils::IdName> &result, std::string &msg);

      bool users(const std::string &user_key, std::vector<utils::IdName> &result, std::string &msg);

      bool techniques(const std::string &user_key, std::vector<utils::IdName> &result, std::string &msg);

      bool column_types(const std::string &user_key, std::vector<utils::IdName> &content, std::string  &msg);

      /**
       * List similars
       */

      bool similar_bio_sources(const std::string name, const std::string &user_key,
                               std::vector<utils::IdName> &result, std::string &msg);

      bool similar_techniques(const std::string name, const std::string &user_key,
                              std::vector<utils::IdName> &result, std::string &msg);

      bool similar_projects(const std::string name, const std::string &user_key,
                            std::vector<utils::IdName> &result, std::string &msg);

      bool similar_epigenetic_marks(const std::string name, const std::string &user_key,
                                    std::vector<utils::IdName> &result, std::string &msg);

      bool similar_genomes(const std::string name, const std::string &user_key,
                           std::vector<utils::IdName> &result, std::string &msg);

      bool similar_experiments(const std::string name, const std::string &genome, const std::string &user_key,
                               std::vector<utils::IdName> &result, std::string &msg);


      bool similar(const std::string &where, const std::string &what,
                   const std::string &user_key, std::vector<utils::IdName> &result, std::string &msg);

      bool similar(const std::string &where, const std::string &field, const std::string &what,
                   const std::string &filter_field, const std::string &filter_what,
                   const std::string &user_key, std::vector<utils::IdName> &result, std::string &msg);
    }
  }
}

#endif