//
//  helpers.hpp
//  epidb
//
//  Created by Felipe Albrecht on 02.07.13.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#ifndef EPIDB_DBA_HELPERS_HPP
#define EPIDB_DBA_HELPERS_HPP

#include <sstream>

#include <mongo/bson/bson.h>
#include <mongo/client/dbclient.h>

#include "../extras/serialize.hpp"
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

      // Get **all** content where the fields match the query parameter, using mongo::Query
      bool get_one(const std::string &where, const mongo::BSONObj &query,
                   mongo::BSONObj &result, std::string &msg);

      // Get **all** content where the fields match the query parameter, using mongo::Query
      bool get_one(const std::string &where, const mongo::Query &query,
                   mongo::BSONObj &result, std::string &msg);

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

      bool collection_size(const std::string &where, unsigned long long &size, std::string &msg);

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


      mongo::BSONArray build_array(const std::vector<std::string> &params);
      mongo::BSONArray build_array(const std::vector<int> &params);
      mongo::BSONArray build_array(const std::vector<serialize::ParameterPtr> &params);
      mongo::BSONArray build_regex_array(const std::vector<std::string> &params);
      mongo::BSONArray build_normalized_array(const std::vector<std::string> &params);
      mongo::BSONArray build_normalized_array(const std::vector<serialize::ParameterPtr> &params);
      mongo::BSONArray build_epigenetic_normalized_array(const std::vector<serialize::ParameterPtr> &params);
      mongo::BSONArray build_annotation_normalized_array(const std::vector<serialize::ParameterPtr> &params);

      std::vector<std::string> build_vector(const std::vector<serialize::ParameterPtr> &params);
      std::vector<std::string> build_vector(const std::vector<mongo::BSONElement> &params);
      std::set<std::string> build_set(const std::vector<mongo::BSONElement> &params);

      std::vector<utils::IdName> bsons_to_id_names(const std::vector<mongo::BSONObj> &bsons);
      utils::IdName bson_to_id_name(const mongo::BSONObj& bson);

      bool check_parameters(const std::vector<serialize::ParameterPtr> &params, const std::function<std::string(const std::string&)> &normalizer, const std::function<bool(const std::string&)> &checker, std::string &wrong);

      bool check_parameters(const std::vector<std::string> &params, const std::function<std::string(const std::string&)> &normalizer, const std::function<bool(const std::string&)> &checker, std::string &wrong);
    }
  }
}

#endif