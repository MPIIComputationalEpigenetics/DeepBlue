//
//  changes.hpp
//  epidb
//
//  Created by Felipe Albrecht on 30.09.14.
//  Copyright (c) 2016 Max Planck Institute for Informatics. All rights reserved.
//

#include <string>

namespace epidb {
  namespace dba {
    namespace changes {
      bool change_extra_metadata(const std::string &id, const std::string &key, const std::string &value, std::string &msg);
    }
  }
}