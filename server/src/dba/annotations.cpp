//
//  annotations.cpp
//  epidb
//
//  Created by Felipe Albrecht on 16.12.14.
//  Copyright (c) 2014 Max Planck Institute for Computer Science. All rights reserved.
//


#include <string>

#include <mongo/bson/bson.h>

#include "annotations.hpp"
#include "collections.hpp"
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
                          std::string &msg)
      {
        if (!helpers::get_increment_counter("datasets", dataset_id, msg) ||
            !helpers::notify_change_occurred("datasets", msg))  {
          return false;
        }

        if (!build_metadata_with_dataset(name, norm_name, genome, norm_genome,
                                         description, norm_description,
                                         extra_metadata_obj,
                                         user_key, ip,
                                         format,
                                         dataset_id,
                                         annotation_id,
                                         annotation_metadata,
                                         msg)) {
          return false;
        }

        return true;
      }


      bool build_metadata_with_dataset(const std::string &name, const std::string &norm_name,
                                       const std::string &genome, const std::string &norm_genome,
                                       const std::string &description, const std::string &norm_description,
                                       const mongo::BSONObj &extra_metadata_obj,
                                       const std::string &user_key, const std::string &ip,
                                       const parser::FileFormat &format,
                                       const int dataset_id,
                                       std::string &annotation_id,
                                       mongo::BSONObj &annotation_metadata,
                                       std::string &msg)
      {
        int a_id;
        if (!helpers::get_increment_counter("annotations", a_id, msg) ||
            !helpers::notify_change_occurred(Collections::ANNOTATIONS(), msg))  {
          return false;
        }
        annotation_id = "a" + utils::integer_to_string(a_id);

        mongo::BSONObjBuilder annotation_data_builder;
        annotation_data_builder.append("_id", annotation_id);
        annotation_data_builder.append(KeyMapper::DATASET(), dataset_id);
        annotation_data_builder.append("name", name);
        annotation_data_builder.append("norm_name", norm_name);
        annotation_data_builder.append("genome", genome);
        annotation_data_builder.append("norm_genome", norm_genome);
        annotation_data_builder.append("description", description);
        annotation_data_builder.append("norm_description", norm_description);
        annotation_data_builder.append("format", format.format());

        annotation_data_builder.append("columns", format.to_bson());

        annotation_data_builder.append("extra_metadata", extra_metadata_obj);

        annotation_metadata = annotation_data_builder.obj();
        return true;
      }
    }
  }
}
