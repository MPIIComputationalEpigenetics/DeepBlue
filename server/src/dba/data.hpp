//
//  data.hpp
//  epidb
//
//  Created by Felipe Albrecht on 06.11.14.
//  Copyright (c) 2016 Max Planck Institute for Informatics. All rights reserved.
//
#ifndef EPIDB_DBA_DATA_HPP
#define EPIDB_DBA_DATA_HPP

#include <string>

#include <mongo/bson/bson.h>

#include "helpers.hpp"

namespace epidb {
  namespace dba {
    namespace data {
      bool sample(const std::string &id, mongo::BSONObj &result, std::string &msg);

      bool genome(const std::string &id, mongo::BSONObj &result, std::string &msg);

      bool project(const std::string &id, const std::vector<std::string>& user_projects,
                   mongo::BSONObj &result, std::string &msg);

      bool technique(const std::string &id, mongo::BSONObj &result, std::string &msg);

      bool biosource(const std::string &id, mongo::BSONObj &result, std::string &msg);

      bool epigenetic_mark(const std::string &id, mongo::BSONObj &result, std::string &msg);

      bool annotation(const std::string &id, mongo::BSONObj &result, std::string &msg);

      bool gene_set(const std::string &id, mongo::BSONObj &result, std::string &msg);

      bool experiment(const std::string &id, const std::vector<std::string>& user_projects,
                      mongo::BSONObj &result, std::string &msg);

      bool query(const std::string &id, mongo::BSONObj &result, std::string &msg);

      bool tiling_region(const std::string &id, mongo::BSONObj &result, std::string &msg);

      bool column_type(const std::string &id, mongo::BSONObj &result, std::string &msg);
    }
  }
}

#endif
