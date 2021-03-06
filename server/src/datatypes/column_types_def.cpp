//
//  column_types_def.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 09.12.14.
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

#include "column_types_def.hpp"

namespace epidb {
  namespace datatypes {

    std::string column_types_names[__COLUMN_TYPES_NR_ITEMS__] = {
      "string",
      "integer",
      "double",
      "range",
      "category",
      "calculated",
      "ERROR"
    };

    std::string column_type_to_name(const COLUMN_TYPES type) {
      if (type > __COLUMN_TYPES_NR_ITEMS__) {
        return "ERROR";
      }
      return column_types_names[type];
    }

    COLUMN_TYPES column_type_name_to_type(const std::string &name)
    {
      if (name == "string") {
        return COLUMN_STRING;
      }
      if (name == "integer") {
        return COLUMN_INTEGER;
      }
      if (name == "double") {
        return COLUMN_DOUBLE;
      }
      if (name == "range") {
        return COLUMN_RANGE;
      }
      if (name == "category") {
        return COLUMN_CATEGORY;
      }
      if (name == "calculated") {
        return COLUMN_CALCULATED;
      }

      return COLUMN_ERR;
    }

    bool compatibility_table[__COLUMN_TYPES_NR_ITEMS__][__COLUMN_TYPES_NR_ITEMS__] = {
      //                String  Integ  Doubl  Range  Categ  Calcu  Error
      /* String   */   { true,  false, false, false, false, false, false},
      /* Integer  */   { false, true,  false, false, false, false, false},
      /* Double   */   { false, false, true,  false, false, false, false},
      /* Range    */   { false, false, true,  true,  false, false, false},
      /* Category */   { true,  false, false, true,  true,  false, false},
      /* Calculated */ { true,  false, false, true,  true,  true,  false},
      /* Error */      { false,  false, false, false, false,  false, false}
    };

    bool column_type_is_compatible(COLUMN_TYPES original, COLUMN_TYPES clone)
    {
      if (original > __COLUMN_TYPES_NR_ITEMS__) {
        return false;
      }
      if (clone > __COLUMN_TYPES_NR_ITEMS__) {
        return false;
      }
      return compatibility_table[original][clone];
    }
  }
}

