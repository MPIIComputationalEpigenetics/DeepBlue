//
//  exists.hpp
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

      bool experiment_set(const std::string &name);

      bool experiment_column(const std::string &experiment_name, const std::string &column_name);

      bool gene_model(const std::string &name);

      bool gene_expression(const std::string &sample_id, const int replica);

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