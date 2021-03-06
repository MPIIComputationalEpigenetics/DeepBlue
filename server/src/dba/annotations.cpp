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

#include "annotations.hpp"
#include "collections.hpp"
#include "helpers.hpp"

#include "../datatypes/metadata.hpp"
#include "../parser/parser_factory.hpp"
#include "../extras/utils.hpp"

namespace epidb {
  namespace dba {
    namespace annotations {

      bool by_name(const std::string &name, const std::string &genome, mongo::BSONObj &annotation, std::string &msg)
      {
        const std::string norm_name = utils::normalize_name(name);
        const std::string norm_genome = utils::normalize_name(genome);

        if (!helpers::get_one(Collections::ANNOTATIONS(),
                              BSON("norm_name" << norm_name << "norm_genome" << norm_genome),
                              annotation)) {
          msg = Error::m(ERR_INVALID_ANNOTATION_NAME, name, genome);
          return false;
        }
        return true;
      }


      bool build_metadata(const std::string &name, const std::string &norm_name,
                          const std::string &genome, const std::string &norm_genome,
                          const std::string &description, const std::string &norm_description,
                          const mongo::BSONObj &extra_metadata_obj,
                          const std::string &ip,
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
                                         ip,
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
                                       const std::string &ip,
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
