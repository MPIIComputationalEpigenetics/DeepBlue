//
//  column_types_def.hpp
//  epidb
//
//  Created by Felipe Albrecht on 08.12.14.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#ifndef COLUMN_TYPES_DEF_HPP
#define COLUMN_TYPES_DEF_HPP

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
    };

    COLUMN_TYPES column_type_name_to_type(const std::string& name);
  }
}

#endif