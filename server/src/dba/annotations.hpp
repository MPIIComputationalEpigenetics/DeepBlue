//
//  annotations.cpp
//  epidb
//
//  Created by Felipe Albrecht on 16.12.14.
//  Copyright (c) 2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include <string>

#include <mongo/bson/bson.h>

#include "helpers.hpp"

#include "../datatypes/metadata.hpp"
#include "../parser/parser_factory.hpp"
#include "../extras/utils.hpp"

namespace epidb {
  namespace dba {
    namespace annotations {
      bool build_metadata(const std::string &name, const std::string &norm_name,
                          const std::string &genome, const std::string &norm_genome,
                          const std::string &description, const std::string &norm_description,
                          const mongo::BSONObj &extra_metadata_obj,
                          const std::string &user_key, const std::string &ip,
                          const parser::FileFormat &format,
                          int &dataset_id,
                          std::string &annotation_id,
                          mongo::BSONObj &annotation_metadata,
                          std::string &msg);

      bool build_metadata_with_dataset(const std::string &name, const std::string &norm_name,
                                       const std::string &genome, const std::string &norm_genome,
                                       const std::string &description, const std::string &norm_description,
                                       const mongo::BSONObj &extra_metadata_obj,
                                       const std::string &user_key, const std::string &ip,
                                       const parser::FileFormat &format,
                                       const int dataset_id,
                                       std::string &annotation_id,
                                       mongo::BSONObj &annotation_metadata,
                                       std::string &msg);
    }
  }
}
