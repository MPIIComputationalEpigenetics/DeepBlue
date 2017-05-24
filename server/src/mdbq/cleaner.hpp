//
//  cleaner.hpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 24.05.17.
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

#include "../datatypes/user.hpp"

#include "common.hpp"

namespace epidb {
  namespace mdbq {
    bool cancel_request(const datatypes::User& user, const std::string& request_id, std::string& msg);
    void remove_result(const std::string request_id);
    bool remove_request_data(const std::string& request_id, TaskState state, std::string& msg);
  }
}