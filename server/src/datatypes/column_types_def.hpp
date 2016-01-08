//
//  column_types_def.hpp
//  DeepBlue Epigenomic Data Server

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
//  Created by Felipe Albrecht on 08.12.14.
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

#ifndef COLUMN_TYPES_DEF_HPP
#define COLUMN_TYPES_DEF_HPP

#include <string>

namespace epidb {
  namespace datatypes {
    enum COLUMN_TYPES {
      COLUMN_STRING,
      COLUMN_INTEGER,
      COLUMN_DOUBLE,
      COLUMN_RANGE,
      COLUMN_CATEGORY,
      COLUMN_CALCULATED,
      COLUMN_ERR,
      __COLUMN_TYPES_NR_ITEMS__
    };

    std::string column_type_to_name(const COLUMN_TYPES type);
    COLUMN_TYPES column_type_name_to_type(const std::string& name);

    bool column_type_is_compatible(COLUMN_TYPES original, COLUMN_TYPES clone);
  }
}

#endif