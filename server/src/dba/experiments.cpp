//
//  experiments.hpp
//  epidb
//
//  Created by Felipe Albrecht on 11.11.14.
//  Copyright (c) 2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include <string>

#include <mongo/bson/bson.h>

#include "collections.hpp"
#include "helpers.hpp"

namespace epidb {
  namespace dba {
    namespace experiments {
      bool by_name(const std::string &name, mongo::BSONObj& experiment, std::string &msg)
      {
        const std::string norm_name = utils::normalize_name(name);
        return helpers::get_one(Collections::EXPERIMENTS(), BSON("norm_name" << norm_name), experiment, msg);
      }
    }
  }
}

