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

      bool experiment_set(const std::string &name)
      {
        return helpers::check_exist(Collections::EXPERIMENT_SETS(), "norm_name", name);
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

      bool annotation(const std::string &name, const std::string &genome)
      {
        mongo::BSONObj query = BSON("norm_name" << name << "norm_genome" << genome);
        return helpers::check_exist(Collections::ANNOTATIONS(), query);
      }

      bool user(const std::string &name)
      {
        return helpers::check_exist(Collections::USERS(), "norm_name", name);
      }

      bool technique(const std::string &name)
      {
        return helpers::check_exist(Collections::TECHNIQUES(), "norm_name", name);
      }

      bool column_type(const std::string &name)
      {
        return helpers::check_exist(Collections::COLUMN_TYPES(), "norm_name", name);
      }

      bool query(const datatypes::User& user, const std::string &query_id, std::string &msg)
      {
        mongo::BSONObj query = BSON("_id" << query_id << "user" << user.id());
        return helpers::check_exist(Collections::QUERIES(), query);
      }
    }
  }
}
