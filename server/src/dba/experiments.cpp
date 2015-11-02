//
//  experiments.hpp
//  epidb
//
//  Created by Felipe Albrecht on 11.11.14.
//  Copyright (c) 2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include <limits>
#include <regex>
#include <string>

#include <mongo/bson/bson.h>

#include "../datatypes/column_types_def.hpp"
#include "../datatypes/regions.hpp"

#include "../errors.hpp"

#include "collections.hpp"
#include "experiments.hpp"
#include "info.hpp"
#include "helpers.hpp"
#include "queries.hpp"

namespace epidb {
  namespace dba {
    namespace experiments {

      bool by_name(const std::string &name, mongo::BSONObj &experiment, std::string &msg)
      {
        const std::string norm_name = utils::normalize_name(name);
        if (!helpers::get_one(Collections::EXPERIMENTS(), BSON("norm_name" << norm_name), experiment)) {
          msg = Error::m(ERR_INVALID_EXPERIMENT_NAME, name);
          return false;
        }
        return true;
      }

      bool by_id(const std::string &id, mongo::BSONObj &experiment, std::string &msg)
      {
        if (!helpers::get_one(Collections::EXPERIMENTS(), BSON("_id" << id), experiment)) {
          msg = Error::m(ERR_INVALID_EXPERIMENT_ID, id);
          return false;
        }
        return true;
      }

      bool get_genome(const std::string &norm_name, std::string &norm_genome, std::string &msg)
      {
        mongo::BSONObj experiment;
        if (!by_name(norm_name, experiment, msg)) {
          return false;
        }
        norm_genome = experiment["norm_genome"].str();

        return true;
      }

      bool get_experiments_names(const std::vector<std::string> &names_ids, std::vector<std::string> &names, std::vector<std::string> &norm_names, std::string &msg)
      {

        for (const auto &name_id : names_ids) {
          if (utils::is_id(name_id, "e")) {
            mongo::BSONObj experiment;
            if (!by_id(name_id, experiment, msg)) {
              return false;
            }
            std::string name = experiment["name"].str();
            names.push_back(name);
            norm_names.push_back(utils::normalize_name(name));
          } else {
            names.push_back(name_id);
            norm_names.push_back(utils::normalize_name(name_id));
          }
        }

        return true;
      }

      bool get_field_pos(const DatasetId &dataset_id, const std::string &column_name, columns::ColumnTypePtr &column_type, std::string &msg)
      {
        processing::StatusPtr status = processing::build_dummy_status();

        if (column_name == "CHROMOSOME") {
          return dba::columns::load_column_type("CHROMOSOME",  status, column_type, msg);
        }

        if (column_name == "START") {
          return dba::columns::load_column_type("START",  status, column_type, msg);
        }

        if (column_name == "END") {
          return dba::columns::load_column_type("END",  status, column_type, msg);
        }

        // TODO: cache experiment_columns
        std::vector<mongo::BSONObj> experiment_columns;
        if (!dba::query::get_columns_from_dataset(dataset_id, experiment_columns, msg)) {
          return false;
        }

        for (auto column : experiment_columns) {
          if (column["name"].String() == column_name) {
            if (!column_type_bsonobj_to_class(column, status, column_type, msg)) {
              return false;
            }
            return true;
          }
        }

        msg = "Invalid column name " + column_name;
        return false;
      }

      bool get_field_pos(const std::string &experiment_name, const std::string &column_name, columns::ColumnTypePtr &column_type,  std::string &msg)
      {
        // TODO: cache experiment
        mongo::BSONObj experiment;
        if (!experiments::by_name(experiment_name, experiment, msg)) {
          return false;
        }
        DatasetId dataset_id = experiment[dba::KeyMapper::DATASET()].Int();

        if (!get_field_pos(dataset_id, column_name, column_type, msg)) {
          return false;
        }

        return true;
      }

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
                          std::string &msg)
      {
        if (!helpers::get_increment_counter("datasets", dataset_id, msg) ||
            !helpers::notify_change_occurred("datasets", msg))  {
          return false;
        }

        if (!build_metadata_with_dataset(name, norm_name,
                                         genome, norm_genome,
                                         epigenetic_mark, norm_epigenetic_mark,
                                         sample_id, technique, norm_technique,
                                         project, norm_project,
                                         description, norm_description,
                                         extra_metadata_obj,
                                         user_key, ip,
                                         format,
                                         dataset_id,
                                         experiment_id,
                                         experiment_metadata,
                                         msg)) {
          return false;
        }

        return true;
      }


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
                                       std::string &msg)
      {
        int e_id;
        if (!helpers::get_increment_counter("experiments", e_id, msg) ||
            !helpers::notify_change_occurred(Collections::EXPERIMENTS(), msg))  {
          return false;
        }
        experiment_id = "e" + utils::integer_to_string(e_id);

        mongo::BSONObjBuilder experiment_data_builder;
        experiment_data_builder.append("_id", experiment_id);
        experiment_data_builder.append(KeyMapper::DATASET(), dataset_id);
        experiment_data_builder.append("name", name);
        experiment_data_builder.append("norm_name", norm_name);
        experiment_data_builder.append("genome", genome);
        experiment_data_builder.append("norm_genome", norm_genome);
        experiment_data_builder.append("epigenetic_mark", epigenetic_mark);
        experiment_data_builder.append("norm_epigenetic_mark", norm_epigenetic_mark);
        experiment_data_builder.append("sample_id", sample_id);
        experiment_data_builder.append("technique", technique);
        experiment_data_builder.append("norm_technique", norm_technique);
        experiment_data_builder.append("project", project);
        experiment_data_builder.append("norm_project", norm_project);
        experiment_data_builder.append("description", description);
        experiment_data_builder.append("norm_description", norm_description);
        experiment_data_builder.append("format", format.format());

        experiment_data_builder.append("columns", format.to_bson());

        experiment_data_builder.append("extra_metadata", extra_metadata_obj);

        std::map<std::string, std::string> sample_data;
        if (!info::get_sample_by_id(sample_id, sample_data, msg, true)) {
          return false;
        }
        mongo::BSONObjBuilder sample_builder;
        std::map<std::string, std::string>::iterator it;
        for (it = sample_data.begin(); it != sample_data.end(); ++it) {
          if ((it->first != "_id") && (it->first != "user")) {
            sample_builder.append(it->first, it->second);
          }
        }
        experiment_data_builder.append("sample_info", sample_builder.obj());

        experiment_metadata = experiment_data_builder.obj();

        return true;
      }

    }
  }
}
