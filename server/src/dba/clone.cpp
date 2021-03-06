//
//  clone.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 07.10.14.
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
#include <vector>

#include <mongo/bson/bson.h>

#include "../parser/parser_factory.hpp"
#include "../extras/utils.hpp"

#include "../connection/connection.hpp"

#include "../datatypes/column_types_def.hpp"
#include "../datatypes/metadata.hpp"

#include "../dba/column_types.hpp"

#include "annotations.hpp"
#include "collections.hpp"
#include "experiments.hpp"
#include "full_text.hpp"
#include "info.hpp"
#include "users.hpp"


#include "../log.hpp"

namespace epidb {
  namespace dba {

    bool check_imutable_columns(const std::string &original_name, const std::string &clone_name, std::string &msg)
    {
      if ((original_name == "CHROMOSOME" || original_name == "START" || original_name == "END") && (original_name != clone_name)) {
        msg = "Column " + original_name + " can not be renamed. Columns CHROMOSOME,START, and END are immutable.";
        return false;
      }
      return true;
    }

    bool check_compatible_type(const columns::ColumnTypePtr &original, const columns::ColumnTypePtr clone, std::string &msg)
    {
      if (!datatypes::column_type_is_compatible(original->type(), clone->type())) {
        msg = "The column '" + clone->name() + "' (type: " + datatypes::column_type_to_name(clone->type()) + ") is incompatible with the original column '" + original->name() + "' (type: " + datatypes::column_type_to_name(original->type()) + ")";
        return false;
      }
      return true;
    }


    bool check_columns(const std::vector<mongo::BSONElement> &original_columns, const std::vector<mongo::BSONElement> &clone_columns, std::string &msg)
    {
      size_t original_columns_size = original_columns.size();
      size_t clone_columns_size = clone_columns.size();

      processing::StatusPtr status = processing::build_dummy_status();

      for (size_t pos = 0; pos < clone_columns_size; pos++) {
        mongo::BSONObj clone_column_bson = clone_columns[pos].Obj();

        if (pos < original_columns_size) {
          mongo::BSONObj original_column_bson = original_columns[pos].Obj();

          columns::ColumnTypePtr original_column;
          if (!columns::column_type_bsonobj_to_class(original_column_bson, status, original_column, msg)) {
            return false;
          }

          columns::ColumnTypePtr clone_column;
          if (!columns::column_type_bsonobj_to_class(clone_column_bson, status, clone_column, msg)) {
            return false;
          }

          if (!check_imutable_columns(original_column->name(), clone_column->name(), msg)) {
            return false;
          }

          if (!check_compatible_type(original_column, clone_column, msg)) {
            return false;
          }

        } else {
          columns::ColumnTypePtr extra_column;
          if (!columns::column_type_bsonobj_to_class(clone_column_bson, status, extra_column, msg)) {
            return false;
          }

          if (extra_column->type() != datatypes::COLUMN_CALCULATED) {
            msg = "Column " + extra_column->name() + " should be calculated. It is not possible to include new columns into cloned experiment/annotation that are not calculated.";
            return false;
          }
        }
      }

      return true;
    }

    bool clone_dataset(const datatypes::User& user,
                       const std::string &dataset_id,
                       const std::string &clone_name, const std::string &norm_clone_name,
                       const std::string &epigenetic_mark, const std::string &norm_epigenetic_mark,
                       const std::string &sample_id,
                       const std::string &technique, const std::string &norm_technique,
                       const std::string &project, const std::string &norm_project,
                       const std::string &description, const std::string &norm_description,
                       const std::string &format, const datatypes::Metadata &extra_metadata,
                       const std::string &ip,
                       std::string &_id, std::string &msg)
    {
      Connection c;

      mongo::BSONObj query = BSON("_id" << dataset_id);

      std::string collection;
      if (dataset_id[0] == 'e') {
        collection = Collections::EXPERIMENTS();
      } else {
        collection = Collections::ANNOTATIONS();
      }

      auto data_cursor = c->query(helpers::collection_name(collection), query, 1);
      mongo::BSONObj original;
      if (data_cursor->more()) {
        original = data_cursor->next().getOwned();
      } else {
        msg = "Internal Error:  experiment with ID " + dataset_id + " not found.";
        c.done();
        return false;
      }

      std::string clone_genome = original["genome"].str();
      std::string clone_norm_genome = original["norm_genome"].str();

      std::string clone_epigenetic_mark;
      std::string clone_norm_epigenetic_mark;
      if (epigenetic_mark.empty()) {
        clone_epigenetic_mark = original["epigenetic_mark"].str();
        clone_norm_genome = original["norm_epigenetic_mark"].str();
      } else {
        clone_epigenetic_mark = epigenetic_mark;
        clone_norm_epigenetic_mark = norm_epigenetic_mark;
      }

      std::string clone_sample_id;
      if (sample_id.empty()) {
        clone_sample_id = original["sample_id"].str();
      } else {
        clone_sample_id = sample_id;
      }


      std::string clone_technique;
      std::string clone_norm_technique;
      if (technique.empty()) {
        clone_technique = original["technique"].str();
        clone_norm_technique = original["norm_technique"].str();
      } else {
        clone_technique = technique;
        clone_norm_technique = norm_technique;
      }


      std::string clone_project;
      std::string clone_norm_project;
      if (project.empty()) {
        clone_project = original["project"].str();
        clone_norm_project = original["norm_project"].str();
      } else {
        clone_project = project;
        clone_norm_project = norm_project;
      }

      // Check if the description should be replaced
      std::string clone_description;
      std::string clone_norm_description;
      if (description.empty()) {
        clone_description = original["description"].str();
        clone_norm_description = original["norm_description"].str();
      } else {
        clone_description = description;
        clone_norm_description = norm_description;
      }

      // Check if the format and columns should be replaced
      parser::FileFormat clone_format;

      std::string original_format = original["format"].str();
      parser::FileFormat original_file_format;
      if (!parser::FileFormatBuilder::build(original_format, original_file_format, msg)) {
        c.done();
        return false;
      }

      if (format.empty()) {
        clone_format = original_file_format;
      } else {
        // Check format

        if (!parser::FileFormatBuilder::build(format, clone_format, msg)) {
          c.done();
          return false;
        }

        if (clone_format.format() != original_file_format.format()) {

          if (clone_format.size() < original_file_format.size()) {
            msg = "The new format has fewer columns than the original format (" + original_format + ")";
            c.done();
            return false;
          }

          std::vector<mongo::BSONElement> original_columns_vector = original["columns"].Array();
          std::vector<mongo::BSONElement> clone_columns_vector;
          mongo::BSONArray clone_format_bson = clone_format.to_bson();
          clone_format_bson.elems(clone_columns_vector);

          if (!check_columns(original_columns_vector, clone_columns_vector, msg)) {
            c.done();
            return false;
          }
        }
      }

      // Check if the extra_metadata should be replaced
      mongo::BSONObj extra_metadata_obj;
      if (!extra_metadata.empty()) {
        extra_metadata_obj = datatypes::metadata_to_bson(extra_metadata);
      } else {
        extra_metadata_obj = original["extra_metadata"].Obj();
      }

      DatasetId internal_dataset_id = original[KeyMapper::DATASET()].Int();
      mongo::BSONObj cloned_metadata;


      if (dataset_id[0] == 'a') {
        if (!annotations::build_metadata_with_dataset(clone_name, norm_clone_name, clone_genome, clone_norm_genome,
            clone_description, clone_norm_description, extra_metadata_obj,
            ip, clone_format,
            internal_dataset_id, _id, cloned_metadata, msg)) {
          return false;
        }

      } else {
        if (!experiments::build_metadata_with_dataset(clone_name, norm_clone_name, clone_genome, clone_norm_genome,
            clone_epigenetic_mark, clone_norm_epigenetic_mark,
            clone_sample_id, clone_technique, clone_norm_technique,
            clone_project, clone_norm_project,
            clone_description, clone_norm_description,
            extra_metadata_obj, ip,
            clone_format, internal_dataset_id,
            _id, cloned_metadata, msg)) {
          return false;
        }
      }

      if (!search::insert_full_text(collection, _id, cloned_metadata, msg)) {
        c.done();
        return false;
      }

      mongo::BSONObjBuilder clone_final_builder;
      clone_final_builder.appendElements(cloned_metadata);


      mongo::BSONObj original_upload_info = original["upload_info"].Obj();

      mongo::BSONObjBuilder upload_info_builder;
      for (auto it = original_upload_info.begin(); it.more(); ) {
        mongo::BSONElement e = it.next();
        if (strncmp(e.fieldName(), "user", strlen("user")) == 0) {
          upload_info_builder.append("user", user.id());
        } else if (strncmp(e.fieldName(), "upload_end", strlen("upload_end")) == 0) {
          upload_info_builder.append("upload_end", mongo::jsTime());
        } else if (strncmp(e.fieldName(), "client_address", strlen("client_address")) == 0) {
          upload_info_builder.append("client_address", ip);
        } else {
          upload_info_builder.append(e);
        }
      }
      clone_final_builder.append("upload_info", upload_info_builder.obj());
      mongo::BSONObj clone = clone_final_builder.obj();

      c->insert(helpers::collection_name(collection), clone);
      if (!c->getLastError().empty()) {
        msg = c->getLastError();
        c.done();
        return false;
      }

      c.done();
      return true;
    }
  }
}
