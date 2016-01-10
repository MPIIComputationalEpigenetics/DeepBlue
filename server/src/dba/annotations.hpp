//
//  annotations.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 16.12.14.
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

      bool build_query_region_set_metadata(const std::string &genome, const std::string &norm_genome,
                                           const std::string &user_key, const std::string &ip,
                                           const parser::FileFormat &format,
                                           int& dataset_id,
                                           std::string &query_region_set_id,
                                           mongo::BSONObj &query_region_set_metadata,
                                           std::string &msg);
    }
  }
}
