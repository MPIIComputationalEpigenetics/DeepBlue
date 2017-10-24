//
//  signature.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 21.10.17.
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

#ifndef _SIGNATURE_HPP_
#define _SIGNATURE_HPP_

#include <bitset>
#include <string>

#include "../datatypes/user.hpp"

#include "../extras/utils.hpp"

namespace epidb {

  namespace signature {

    bool list_similar_experiments(const datatypes::User& user, const std::string& query_id, const std::vector<utils::IdName>& names,
                                  processing::StatusPtr status,
                                  std::vector<utils::IdNameCount> results, std::string& msg);
  }
}


#endif