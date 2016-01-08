//
//  experiments.hpp
//  epidb
//
//  Created by Felipe Albrecht on 11.11.14.
//  Copyright (c) 2016 Max Planck Institute for Informatics. All rights reserved.
//

#ifndef EPIDB_DBA_EXPERIMENTS_HPP
#define EPIDB_DBA_EXPERIMENTS_HPP

#include <string>

#include <mongo/bson/bson.h>

#include "../datatypes/column_types_def.hpp"
#include "../datatypes/metadata.hpp"

#include "../extras/utils.hpp"

#include "../parser/parser_factory.hpp"

namespace epidb {
  namespace dba {
    namespace experiments {
      bool by_name(const std::string &name, mongo::BSONObj &experiment, std::string &msg);

      bool by_id(const std::string &id, mongo::BSONObj &experiment, std::string &msg);

      bool get_genome(const std::string &norm_name, std::string &norm_genome, std::string &msg);

      bool get_experiments_names(const std::vector<std::string> &names_ids, std::vector<std::string> &names, std::vector<std::string> &norm_names, std::string &msg);

      bool get_field_pos(const DatasetId &dataset_id, const std::string &column_name, columns::ColumnTypePtr &column_type, std::string &msg);

      bool get_field_pos(const std::string &experiment_name, const std::string &column_name, columns::ColumnTypePtr &column_type,  std::string &msg);

      bool build_metadata(const std::string &name, const std::string &norm_name,
                          const std::string &genome, const std::string &norm_genome,
                          const std::string &epigenetic_mark, const std::string &norm_epigenetic_mark,
                          const std::string &sample_id, const std::string &technique, const std::string &norm_technique,
                          const std::string &project, const std::string &norm_project,
                          const std::string &description, const std::string &norm_description,
                          const mongo::BSONObj &extra_metadata_obj,
                          const std::string &user_key, const std::string &ip,
                          const parser::FileFormat &format,
                          int &dataset_id,
                          std::string &experiment_id,
                          mongo::BSONObj &experiment_metadata,
                          std::string &msg);

      bool build_metadata_with_dataset(const std::string &name, const std::string &norm_name,
                                       const std::string &genome, const std::string &norm_genome,
                                       const std::string &epigenetic_mark, const std::string &norm_epigenetic_mark,
                                       const std::string &sample_id, const std::string &technique, const std::string &norm_technique,
                                       const std::string &project, const std::string &norm_project,
                                       const std::string &description, const std::string &norm_description,
                                       const mongo::BSONObj &extra_metadata_obj,
                                       const std::string &user_key, const std::string &ip,
                                       const parser::FileFormat &format,
                                       const int dataset_id,
                                       std::string &experiment_id,
                                       mongo::BSONObj &experiment_metadata,
                                       std::string &msg);

      // TODO: move others functions from dba.hpp to here
    }
  }
}

#endif