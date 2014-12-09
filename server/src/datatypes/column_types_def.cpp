//
//  column_types_def.cpp
//  epidb
//
//  Created by Felipe Albrecht on 09.12.14.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include <string>

#include "column_types_def.hpp"

namespace epidb {
  namespace datatypes {
    COLUMN_TYPES column_type_name_to_type(const std::string& name) {
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
      return COLUMN_ERR;
    }
  }
}

