//
//  helpers.hpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 02.07.13.
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

#ifndef EPIDB_DBA_HELPERS_HPP
#define EPIDB_DBA_HELPERS_HPP

#include <sstream>

#include <mongo/bson/bson.h>
#include <mongo/client/dbclient.h>

#include "../extras/utils.hpp"

namespace epidb {
  namespace dba {
    namespace helpers {

      typedef std::pair<std::string, std::string> QueryPair;

      const std::string collection_name(const std::string &name);

      const std::string region_collection_name(const std::string &genome, const std::string &chromosome);

      // Get **all** content from **entire** colection
      bool get(const std::string &where,
               std::vector<std::string> &result, std::string &msg);

      // Get **all** content where the 'field' has the 'expected_content'
      bool get(const std::string &where, const std::string &field, const std::string &expected_content,
               std::vector<mongo::BSONObj> &result, std::string &msg);

      // Get returned_field field content from all elements from the collection
      bool get(const std::string &where, const std::string &returned_field,
               std::vector<std::string> &result, std::string &msg);

      // Get **all** content where the fields match the conditions
      bool get(const std::string &where, const std::vector<QueryPair> &conditions,
               std::vector<mongo::BSONObj> &results, std::string &msg);

      // Get **all** content where the fields match the query parameter
      bool get(const std::string &where, const mongo::BSONObj &query,
               std::vector<mongo::BSONObj> &results, std::string &msg);

      // Get **all** content where the fields match the query parameter, using mongo::Query
      bool get(const std::string &where, const mongo::Query &query,
               std::vector<mongo::BSONObj> &results, std::string &msg);

      // Return true and one element that match the given query.
      // Return false if does not find any element.
      bool get_one(const std::string &where, const mongo::BSONObj &query,
                   mongo::BSONObj &result);

      // Return true and one element that match the given query.
      // Return false if does not find any element.
      bool get_one(const std::string &where, const mongo::Query &query,
                   mongo::BSONObj &result);

      // Get Id and Name from the given collection documents and a filter BSon
      bool get(const std::string &where, mongo::BSONObj filter,
               std::vector<utils::IdName> &results, std::string &msg);

      // Get Id and Name from the given collection documents.
      bool get(const std::string &where, std::vector<utils::IdName> &results, std::string &msg);

      // Get the content from the field where from where the query match
      bool get(const std::string &where, const mongo::Query &query, const std::vector<std::string> &fields,
               std::vector<mongo::BSONObj> &results, std::string &msg);

      bool get_name(const std::string &where, const std::string &norm_name,
                    utils::IdName &id_name, std::string &msg);

      bool get_id(const std::string &where, const std::string &norm_name,
                  std::string &id, std::string &msg);

      bool check_exist(const std::string &where, const std::string &field, const std::string &content);

      bool check_exist(const std::string &where, const std::string &field, const bool content);

      bool check_exist(const std::string &where, const mongo::BSONObj& query);

      bool remove_one(const std::string &collection, const std::string &id, std::string &msg, const std::string &field = "_id");

      bool remove_all(const std::string &collection, const mongo::Query &query, std::string &msg);

      bool remove_collection(const std::string &collection, std::string &msg);

      bool collection_size(const std::string &where, const mongo::BSONObj& query, size_t &size, std::string &msg, const bool check=true);

      mongo::BSONArray build_dataset_ids_arrays(const std::string &where, const mongo::BSONObj& query);

      template<typename T>
      mongo::BSONObj build_condition_array(const std::vector<T> &params, const std::string &condition)
      {
        mongo::BSONArrayBuilder ab;
        for (const auto& param :  params) {
          ab.append(param);
        }
        mongo::BSONObjBuilder in_condition;
        in_condition.append (condition, ab.arr());
        return in_condition.obj();
      }


      /**
      * \brief  Increment counter variable from Database by one
      * \param  name  Name of counter-variable
      *         count Return: Updated counter value
      */
      bool get_increment_counter(const std::string &name, int &id, std::string &msg);

      /**
      * \brief  Get counter variable from Database
      * \param  name  Name of counter-variable
      *         count Return: Counter value
      */
      bool get_counter(const std::string &name, int &count, std::string &msg);

      /**
      * \brief  To be called if a change occurred in the stored data
      * \param  name  Type of data
      */
      bool notify_change_occurred(const std::string &name, std::string &msg);
    }
  }
}

#endif