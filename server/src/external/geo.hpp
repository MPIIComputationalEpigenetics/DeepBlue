//
//  geo.hpp
//  epidb
//
//  Created by Felipe Albrecht on 10.12.13.
//  Copyright (c) 2016 Max Planck Institute for Informatics. All rights reserved.
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
