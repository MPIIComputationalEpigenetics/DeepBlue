//
//  geo.hpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 10.12.13.
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

#ifndef EPIDB_EXTERNA_GEO_HPP
#define EPIDB_EXTERNA_GEO_HPP

#include "../datatypes/metadata.hpp"

namespace epidb {
  namespace external {
    namespace geo {
      bool load_gsm(const std::string &gsm_id, datatypes::Metadata &metadata, std::string& msg);
    }
  }
}

#endif
