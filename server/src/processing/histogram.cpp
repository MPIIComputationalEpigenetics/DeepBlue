//
//  histogram.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 29.12.14.
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
      bool histogram(const std::string& query_id, const std::string& column_name, const int bars, const std::string& user_key, processing::StatusPtr status, mongo::BSONArray& counts, std::string& msg) {
          return true;
      }
    }
}