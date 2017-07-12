//
//  datatable.hpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 22.03.2016.
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

#ifndef EPIDB_DBA_DATATABLE_HPP
#define EPIDB_DBA_DATATABLE_HPP

#include <string>
#include <vector>

#include "../datatypes/metadata.hpp"
#include "../datatypes/user.hpp"


namespace epidb {
  namespace dba {
    namespace datatable {

      bool datatable(const datatypes::User& user,
                     const std::string collection, const std::vector<std::string> columns,
                     const long long start, const long long length,
                     const std::string& global_search, const std::string& sort_column, const std::string& sort_direction,
                     const bool has_filter, const datatypes::Metadata& columns_filters,
                     size_t& total_elements, std::vector<std::vector<std::string>>& results,
                     std::string& msg);
    }
  }
}

#endif