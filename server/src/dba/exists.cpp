//
//  exists.cpp
//  epidb
//
//  Created by Felipe Albrecht on 12.02.15.
//  Copyright (c) 2015 Max Planck Institute for Computer Science. All rights reserved.
//

#include <string>

#include <mongo/bson/bson.h>

#include "exists.hpp"

#include "collections.hpp"
#include "helpers.hpp"
#include "users.hpp"

namespace epidb {
  namespace dba {
    namespace exists {

      bool genome(const std::string &name)
      {
        return helpers::check_exist(Collections::GENOMES(), "norm_name", name);
      }

      bool biosource(const std::string &name)
      {
        return helpers::check_exist(Collections::BIOSOURCES(), "norm_name", name);
      }

      bool biosource_synonym(const std::string &name)
      {
        return helpers::check_exist(Collections::BIOSOURCE_SYNONYM_NAMES(), "norm_synonym", name);
      }

      bool sample(const std::string &id)
      {
        return helpers::check_exist(Collections::SAMPLES(), "_id", id);
      }

      bool project(const std::string &name)
      {
        return helpers::check_exist(Collections::PROJECTS(), "norm_name", name);
      }

      bool epigenetic_mark(const std::string &name)
      {
        return helpers::check_exist(Collections::EPIGENETIC_MARKS(), "norm_name", name);
      }

      bool experiment(const std::string &name)
      {
        return helpers::check_exist(Collections::EXPERIMENTS(), "norm_name", name);
      }

      bool annotation(const std::string &name, const std::string &genome)
      {
        mongo::BSONObj query = BSON("norm_name" << name << "norm_genome" << genome);
        return helpers::check_exist(Collections::ANNOTATIONS(), query);
      }

      bool user(const std::string &name)
      {
        return helpers::check_exist(Collections::USERS(), "norm_name", name);
      }

      bool user_by_key(const std::string &user_key)
      {
        return helpers::check_exist(Collections::USERS(), "key", user_key);
      }

      bool technique(const std::string &name)
      {
        return helpers::check_exist(Collections::TECHNIQUES(), "norm_name", name);
      }

      bool column_type(const std::string &name)
      {
        return helpers::check_exist(Collections::COLUMN_TYPES(), "norm_name", name);
      }

      bool query(const std::string &query_id, const std::string &user_key, std::string &msg)
      {
        utils::IdName id_user_name;
        if (!users::get_user_name(user_key, id_user_name, msg)) {
          return false;
        }

        if (id_user_name.name.empty()) {
          return false;
        }

        mongo::BSONObj query = BSON("_id" << query_id << "user" << id_user_name.name);
        return helpers::check_exist(Collections::QUERIES(), query);
      }
    }
  }
}
