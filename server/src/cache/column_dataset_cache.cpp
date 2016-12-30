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

#include "../datatypes/regions.hpp"

#include "../dba/queries.hpp"

#include "column_dataset_cache.hpp"

namespace epidb {
  namespace cache {

    dataset_columnn_cache::QUERY_RESULT fn_load_dataset_columns(const dataset_columnn_cache::QUERY_KEY& qk)
    {
      ChromosomeRegionsList regions;
      std::string msg;

      dataset_columnn_cache::QUERY_RESULT result;
      result.success  =  dba::query::get_columns_from_dataset(qk.dataset_id, result.columns, result.msg);

      return result;
    }

    lru_cache_using_boost<dataset_columnn_cache::QUERY_KEY, dataset_columnn_cache::QUERY_RESULT, boost::bimaps::set_of> DATASET_QUERY_CACHE(fn_load_dataset_columns, 4096);


    bool get_columns_from_dataset(const DatasetId & dataset_id, std::vector<mongo::BSONObj> &columns, std::string & msg)
    {
      dataset_columnn_cache::QUERY_KEY qk;
      qk.dataset_id = dataset_id;

      dataset_columnn_cache::QUERY_RESULT qr = DATASET_QUERY_CACHE(qk);

      columns = std::move(qr.columns);
      msg = qr.msg;

      return qr.success;
    }
  }
}
