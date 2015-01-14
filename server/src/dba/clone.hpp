//
//  clone.hpp
//  epidb
//
//  Created by Felipe Albrecht on 07.10.14.
//  Copyright (c) 2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include <map>
#include <string>
#include <vector>

#include "../datatypes/metadata.hpp"

#include "../parser/parser_factory.hpp"

namespace epidb {
  namespace dba {
    bool clone_dataset(const std::string &dataset_id, const std::string &name, const std::string &norm_name,
                       const std::string &epigenetic_mark, const std::string &norm_epigenetic_mark,
                       const std::string &sample_id,
                       const std::string &technique, const std::string &norm_technique,
                       const std::string &project, const std::string &norm_project,
                       const std::string &description, const std::string &norm_description,
                       const std::string &format, const datatypes::Metadata &extra_metadata,
                       const std::string user_key, const std::string& ip,
                       std::string &_id, std::string &msg);

  }
}