//
//  column_dataset_cache.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 30.12.2016.
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

#include <boost/bimap/set_of.hpp>

#include "../algorithms/lru.hpp"

#include "../dba/column_types.hpp"
#include "../dba/queries.hpp"

#include "../extras/utils.hpp"

#include "column_dataset_cache.hpp"

#include "../errors.hpp"

namespace epidb {
  namespace cache {

    dataset_id_bson::QUERY_RESULT fn_load_bson_by_dataset_id(const dataset_id_bson::QUERY_KEY& qk)
    {
      std::string msg;

      dataset_id_bson::QUERY_RESULT result;
      result.success  =  dba::query::__get_bson_by_dataset_id(qk.dataset_id, result.obj, result.msg);

      return result;
    }

    lru_cache_using_boost<
      dataset_id_bson::QUERY_KEY,
      dataset_id_bson::QUERY_RESULT,
      boost::bimaps::set_of> DATASET_ID_BSON_CACHE(fn_load_bson_by_dataset_id, 4096);


    bool get_bson_by_dataset_id(DatasetId dataset_id, mongo::BSONObj &obj, std::string &msg)
    {
      dataset_id_bson::QUERY_KEY qk;
      qk.dataset_id = dataset_id;

      dataset_id_bson::QUERY_RESULT qr = DATASET_ID_BSON_CACHE(qk);

      obj = qr.obj;
      msg = qr.msg;

      return qr.success;
    }

    // --

    dataset_columnn_cache::QUERY_RESULT fn_load_dataset_columns(const dataset_columnn_cache::QUERY_KEY& qk)
    {
      std::string msg;

      dataset_columnn_cache::QUERY_RESULT result;
      result.success  =  dba::query::__get_columns_from_dataset(qk.dataset_id, result.columns, result.msg);

      return result;
    }

    lru_cache_using_boost<
      dataset_columnn_cache::QUERY_KEY,
      dataset_columnn_cache::QUERY_RESULT,
      boost::bimaps::set_of> DATASET_QUERY_CACHE(fn_load_dataset_columns, 4096);


    bool get_columns_from_dataset(const DatasetId & dataset_id, std::vector<mongo::BSONObj> &columns, std::string & msg)
    {
      dataset_columnn_cache::QUERY_KEY qk;
      qk.dataset_id = dataset_id;

      dataset_columnn_cache::QUERY_RESULT qr = DATASET_QUERY_CACHE(qk);

      columns = std::move(qr.columns);
      msg = qr.msg;

      return qr.success;
    }

    // --

    column_position_dataset::QUERY_RESULT fn_load_column_position_from_dataset(const column_position_dataset::QUERY_KEY& qk)
    {
      column_position_dataset::QUERY_RESULT result;

      std::vector<mongo::BSONObj> columns;
      if (!cache::get_columns_from_dataset(qk.dataset_id, columns, result.msg)) {
        result.success = false;
        return result;
      }
      for (const auto& c : columns) {
        if (c["name"].str() == qk.column_name) {
          result.pos = c["pos"].Number();
          result.success = true;
          return result;
        }
      }

      mongo::BSONObj obj;
      if (!get_bson_by_dataset_id(qk.dataset_id, obj, result.msg)) {
        return result;
      }

      // Give a nice error message when we do not find the column in the given dataset.
      result.success = false;
      result.msg = Error::m(ERR_INVALID_EXPERIMENT_COLUMN, obj["name"].str(), qk.column_name);
      result.pos = -1;

      return result;
    }

    lru_cache_using_boost<
      column_position_dataset::QUERY_KEY,
      column_position_dataset::QUERY_RESULT,
      boost::bimaps::set_of> COLUMN_POSITION_DATASET_CACHE(fn_load_column_position_from_dataset, 4096);


    bool get_column_position_from_dataset(const DatasetId & dataset_id, const std::string &column_name,  int& pos, std::string & msg)
    {
      column_position_dataset::QUERY_KEY qk;
      qk.dataset_id = dataset_id;
      qk.column_name = column_name;

      column_position_dataset::QUERY_RESULT qr = COLUMN_POSITION_DATASET_CACHE(qk);

      pos = qr.pos;
      msg = qr.msg;

      return qr.success;
    }

    // --

    column_type_dataset::QUERY_RESULT fn_load_column_type_from_dataset(const column_type_dataset::QUERY_KEY& qk)
    {
      const std::string& column_name = qk.column_name;
      DatasetId dataset_id = qk.dataset_id;

      column_type_dataset::QUERY_RESULT result;

      processing::StatusPtr status = processing::build_dummy_status();

      if ((column_name == "CHROMOSOME") || (column_name == "START") || (column_name == "END")) {
        result.success = dba::columns::load_column_type(column_name,  status, result.column_type, result.msg);
        return result;
      }

      std::vector<mongo::BSONObj> experiment_columns;
      if (!cache::get_columns_from_dataset(dataset_id, experiment_columns, result.msg)) {
        result.success = false;
        return result;
      }

      for (auto column : experiment_columns) {
        if (column["name"].String() == column_name) {
          if (!column_type_bsonobj_to_class(column, status, result.column_type, result.msg)) {
            result.success = false;
            return result;
          }
          result.success = true;
          return result;
        }
      }

      result.msg = "Invalid column name " + column_name;
      result.success = false;
      return result;
    }

    lru_cache_using_boost<
      column_type_dataset::QUERY_KEY,
      column_type_dataset::QUERY_RESULT,
      boost::bimaps::set_of> COLUMN_TYPE_DATASET_CACHE(fn_load_column_type_from_dataset, 4096);


    bool get_column_type_from_dataset(const DatasetId &dataset_id, const std::string &column_name, dba::columns::ColumnTypePtr& column_type, std::string &msg)
    {
      column_type_dataset::QUERY_KEY qk;
      qk.dataset_id = dataset_id;
      qk.column_name = column_name;

      column_type_dataset::QUERY_RESULT qr = COLUMN_TYPE_DATASET_CACHE(qk);

      column_type = qr.column_type;
      msg = qr.msg;

      return qr.success;
    }

    void column_dataset_cache_invalidate() {
      DATASET_ID_BSON_CACHE.clear();
      DATASET_QUERY_CACHE.clear();
      COLUMN_POSITION_DATASET_CACHE.clear();
      COLUMN_TYPE_DATASET_CACHE.clear();
    }
  }
}
