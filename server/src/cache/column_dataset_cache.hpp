//
//  column_dataset_cache.hpp
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

namespace epidb {
  namespace cache {
    namespace dataset_columnn_cache {
      struct QUERY_KEY {
        DatasetId dataset_id;

        int operator<(const QUERY_KEY& rhs) const
        {
          return dataset_id < rhs.dataset_id;
        }
      };

      struct QUERY_RESULT {
        bool success;
        std::vector<mongo::BSONObj> columns;
        std::string msg;
      };
    }

    namespace column_position_dataset {
      struct QUERY_KEY {
        DatasetId dataset_id;
        const std::string column_name;

        int operator<(const QUERY_KEY& rhs) const
        {
          return dataset_id < rhs.dataset_id && column_name.compare(rhs.column_name);
        }
      };

      struct QUERY_RESULT {
        bool success;
        int pos;
        std::string msg;
      };
    }


    bool get_columns_from_dataset(const DatasetId & dataset_id, std::vector<mongo::BSONObj> &columns, std::string & msg);

    bool get_column_position_from_dataset(const DatasetId & dataset_id, const std::string &name,  int& pos, std::string & msg);
  }
}