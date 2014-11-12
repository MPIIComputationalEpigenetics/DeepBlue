//
//  experiments.hpp
//  epidb
//
//  Created by Felipe Albrecht on 11.11.14.
//  Copyright (c) 2014 Max Planck Institute for Computer Science. All rights reserved.
//

#ifndef EPIDB_DBA_EXPERIMENTS_HPP
#define EPIDB_DBA_EXPERIMENTS_HPP

#include <string>

#include "../extras/utils.hpp"

namespace epidb {
  namespace dba {
    namespace experiments {
      bool by_name(const std::string &name, mongo::BSONObj& experiment, std::string &msg);

      // TODO: move others functions from dba.hpp to here
    }
  }
}

#endif