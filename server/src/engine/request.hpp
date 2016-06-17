//
//  request.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 27.01.15.
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

#ifndef REQUEST_STATUS_HPP
#define REQUEST_STATUS_HPP

#include <vector>

#include "../datatypes/metadata.hpp"
#include "../extras/utils.hpp"

namespace epidb {
  namespace request {

    typedef struct Status {
      std::string state;
      std::string message;
    } Status;

    typedef struct Job {
      std::string _id;
      Status status;
      boost::posix_time::ptime create_time;
      std::string command;
      std::string user_id;
      datatypes::Metadata misc;
      /*
      * \brief Time at which the job was finished. Only to be set, if status == TS_DONE
      */
      boost::posix_time::ptime finish_time;
      std::string query_id;
    } Job;
  }
}

#endif