//
//  count_regions.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 28.01.14.
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

#include "../dba/queries.hpp"

namespace epidb {
  namespace processing {

    bool count_regions(const datatypes::User& user,
                       const std::string &query_id,
                       processing::StatusPtr status, size_t &count, std::string &msg)
    {
      IS_PROCESSING_CANCELLED(status);
      processing::RunningOp runningOp =  status->start_operation(PROCESS_COUNT);

      if (!dba::query::count_regions(user, query_id, status, count, msg)) {
        return false;
      }
      return true;
    }

  }
}