//
//  list.hpp
//  DeepBlue Epigenomic Data Server

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
//  Created by Felipe Albrecht on 07.04.14.
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

#ifndef EPIDB_DBA_LIST_HPP
#define EPIDB_DBA_LIST_HPP

#include <string>
#include <unordered_map>
#include <vector>

#include <mongo/bson/bson.h>
#include <mongo/client/dbclient.h>

#include "../datatypes/metadata.hpp"

#include "../extras/utils.hpp"

namespace epidb {
  namespace dba {
    namespace list {

      /**
       * Lists
       */

      bool genomes(const std::string &user_key, std::vector<utils::IdName> &result, std::string &msg);

      bool biosources(const datatypes::Metadata &metadata,
                      std::vector<utils::IdName> &result, std::string &msg);

      bool samples(const std::string &user_key, const mongo::BSONArray &biosources,
                   const datatypes::Metadata &metadata,
                   std::vector<std::string> &result, std::string &msg);

      bool private_projects(const std::string &user_key, std::vector<utils::IdName> &result, std::string &msg);

      bool projects(const std::string &user_key, std::vector<utils::IdName> &result, std::string &msg);

      bool epigenetic_marks(const datatypes::Metadata &metadata,
                            std::vector<utils::IdName> &result, std::string &msg);

      bool experiments(const mongo::BSONObj query, std::vector<utils::IdName> &result, std::string &msg);

      bool experiments(const mongo::Query query, std::vector<utils::IdName> &result, std::string &msg);

      bool annotations(const std::string &genome, const std::string &user_key,
                       std::vector<utils::IdName> &result, std::string &msg);

      bool annotations(const std::string &user_key, std::vector<utils::IdName> &result, std::string &msg);

      bool users(const std::string &user_key, std::vector<utils::IdName> &result, std::string &msg);

      bool techniques(const std::string &user_key, std::vector<utils::IdName> &result, std::string &msg);

      bool column_types(const std::string &user_key, std::vector<utils::IdName> &content, std::string  &msg);

      /**
       * List similars
       */

      bool similar_biosources(const std::string &name, const std::string &user_key,
                              std::vector<utils::IdName> &result, std::string &msg);

      bool similar_techniques(const std::string &name, const std::string &user_key,
                              std::vector<utils::IdName> &result, std::string &msg);

      bool similar_projects(const std::string &name, const std::string &user_key,
                            std::vector<utils::IdName> &result, std::string &msg);

      bool similar_epigenetic_marks(const std::string &name, const std::string &user_key,
                                    std::vector<utils::IdName> &result, std::string &msg);

      bool similar_genomes(const std::string &name, const std::string &user_key,
                           std::vector<utils::IdName> &result, std::string &msg);

      bool similar_experiments(const std::string &name, const std::string &genome, const std::string &user_key,
                               std::vector<utils::IdName> &result, std::string &msg);


      bool similar(const std::string &where, const std::string &what,
                   const std::string &user_key, std::vector<utils::IdName> &result, std::string &msg,
                   const size_t total = 20);

      bool similar(const std::string &where, const std::string &field, const std::string &what,
                   const std::string &filter_field, const std::string &filter_what,
                   const std::string &user_key, std::vector<utils::IdName> &result, std::string &msg,
                   const size_t total = 20);


      bool build_list_experiments_query(const std::vector<serialize::ParameterPtr> genomes, const std::vector<serialize::ParameterPtr> types,
                                        const std::vector<serialize::ParameterPtr> epigenetic_marks, const std::vector<serialize::ParameterPtr> biosources,
                                        const std::vector<serialize::ParameterPtr> sample_ids, const std::vector<serialize::ParameterPtr> techniques,
                                        const std::vector<serialize::ParameterPtr> projects, const std::string user_key,
                                        mongo::BSONObj& query, std::string msg);

      /**
       * List in use
       */

      bool in_use(const std::string &collection, const std::string &field_name, const std::string &user_key,
                  std::vector<utils::IdNameCount> &names, std::string &msg);


      bool faceting(const mongo::BSONObj& experimentsq_uery, const std::string &user_key,
                    std::unordered_map<std::string, std::vector<utils::IdNameCount> > &result,
                    std::string &msg);
    }
  }
}

#endif