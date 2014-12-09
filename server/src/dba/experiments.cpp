//
//  experiments.hpp
//  epidb
//
//  Created by Felipe Albrecht on 11.11.14.
//  Copyright (c) 2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include <string>

#include <mongo/bson/bson.h>

#include "../datatypes/column_types_def.hpp"

#include "collections.hpp"
#include "helpers.hpp"
#include "queries.hpp"

#include "../regions.hpp"

namespace epidb {
  namespace dba {
    namespace experiments {
      bool by_name(const std::string &name, mongo::BSONObj &experiment, std::string &msg)
      {
        const std::string norm_name = utils::normalize_name(name);
        return helpers::get_one(Collections::EXPERIMENTS(), BSON("norm_name" << norm_name), experiment, msg);
      }

      bool get_field_pos(const DatasetId &dataset_id, const std::string &column_name, int &pos, datatypes::COLUMN_TYPES &type, std::string &msg)
      {
        pos = -1;
        type = datatypes::COLUMN_ERR;


        if (column_name == "CHROMOSOME") {
          type = datatypes::COLUMN_STRING;
          return true;
        }

        if (column_name == "START" || column_name == "END") {
          type = datatypes::COLUMN_INTEGER;
          return true;
        }

        // TODO: cache experiment_columns
        std::vector<mongo::BSONObj> experiment_columns;
        if (!dba::query::get_columns_from_dataset(dataset_id, experiment_columns, msg)) {
          return false;
        }

        for (auto column : experiment_columns) {
          if (column["name"].String() == column_name) {
            pos = column["pos"].Int();
            const std::string& type_name = column["column_type"].str();
            type = datatypes::column_type_name_to_type(type_name);
            break;
          }
        }

        if (pos == -1) {
          msg = "Invalid column name " + column_name;
          return false;
        }

        return true;
      }

      bool get_field_pos(const std::string &experiment_name, const std::string &column_name, int &pos, datatypes::COLUMN_TYPES &type, std::string &msg)
      {
        pos = -1;

        // TODO: cache experiment
        mongo::BSONObj experiment;
        if (!experiments::by_name(experiment_name, experiment, msg)) {
          return false;
        }
        DatasetId dataset_id = experiment[dba::KeyMapper::DATASET()].Int();

        if (!get_field_pos(dataset_id, column_name, pos, type, msg)) {
          return false;
        }

        if (pos == -1) {
          msg = "Invalid column name " + column_name + " for the experiment " + experiment_name;
          return false;
        }

        return true;
      }
    }
  }
}

