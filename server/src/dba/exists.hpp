//
//  exists.hpp
//  epidb
//
//  Created by Felipe Albrecht on 12.02.15.
//  Copyright (c) 2015 Max Planck Institute for Computer Science. All rights reserved.
//

#ifndef EPIDB_DBA_EXISTS_HPP
#define EPIDB_DBA_EXISTS_HPP

#include <string>

#include <mongo/bson/bson.h>
#include <mongo/client/dbclient.h>

#include "dba.hpp"

#include "../datatypes/metadata.hpp"

#include "../extras/utils.hpp"

namespace epidb {
  namespace dba {
    namespace exists {

      /**
       * Exists
       */

      bool genome(const std::string &name);

      bool biosource(const std::string &name);

      bool biosource_synonym(const std::string &name);

      bool sample(const std::string &id);

      bool project(const std::string &name);

      bool epigenetic_mark(const std::string &name);

      bool experiment(const std::string &name);

      bool gene_set(const std::string &name);

      bool annotation(const std::string &name, const std::string& genome);

      bool user(const std::string &name);

      bool user_by_key(const std::string &user_key);

      bool technique(const std::string &name);

      bool column_type(const std::string &name);

      bool query(const std::string &query_id, const std::string &user_key, std::string& msg);
    }
  }
}

#endif