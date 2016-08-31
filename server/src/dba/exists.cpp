//
//  exists.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 12.02.15.
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

      bool experiment_column(const std::string &experiment_name, const std::string &column_name)
      {
        mongo::BSONObj query = BSON("norm_name" << experiment_name << "columns.name" << column_name);
        return helpers::check_exist(Collections::EXPERIMENTS(), query);
      }

      bool gene_model(const std::string &name)
      {
        return helpers::check_exist(Collections::GENE_MODELS(), "norm_name", name);
      }

      bool gene_expression(const std::string &sample_id, const int replica)
      {
        return helpers::check_exist(Collections::GENE_EXPRESSIONS(),
                                    BSON("sample_id" << sample_id << "replica" << replica));
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
        utils::IdName user;
        if (!users::get_user(user_key, user, msg)) {
          return false;
        }

        if (user.name.empty()) {
          return false;
        }

        mongo::BSONObj query = BSON("_id" << query_id << "user" << user.id);
        return helpers::check_exist(Collections::QUERIES(), query);
      }
    }
  }
}
