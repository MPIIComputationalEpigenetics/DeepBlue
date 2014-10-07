//
//  clone.cpp
//  epidb
//
//  Created by Felipe Albrecht on 07.10.14.
//  Copyright (c) 2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include <string>
#include <vector>

#include <mongo/bson/bson.h>
#include <mongo/client/dbclient.h>

#include "../parser/parser_factory.hpp"
#include "../extras/utils.hpp"

#include "../datatypes/metadata.hpp"

#include "collections.hpp"
#include "config.hpp"
#include "full_text.hpp"
#include "helpers.hpp"

#include "../log.hpp"

namespace epidb {
  namespace dba {


    bool clone_dataset(const std::string &dataset_id, const std::string &name, const std::string &norm_name,
                       const std::string &description, const std::string &norm_description,
                       const parser::FileFormat &format,
                       const datatypes::Metadata &extra_metadata,
                       std::string &_id,
                       std::string &msg)
    {

      mongo::ScopedDbConnection c(config::get_mongodb_server());

      mongo::BSONObj query = BSON("_id" << dataset_id);

      std::string collection;
      if (dataset_id[0] == 'e') {
        collection = Collections::EXPERIMENTS();
      } else {
        collection = Collections::ANNOTATIONS();
      }

      std::auto_ptr<mongo::DBClientCursor> data_cursor = c->query(helpers::collection_name(collection), query, 1);
      mongo::BSONObj original;
      if (data_cursor->more()) {
        original = data_cursor->next().getOwned();
      } else {
        msg = "experiment with id " + dataset_id + " not found.";
        c.done();
        return false;
      }

      // Check if the name should be replaced
      std::string clone_name;
      std::string clone_norm_name;
      if (description.empty()) {
        clone_name = original["name"].str();
        clone_norm_name = original["norm_name"].str();
      } else {
        clone_name = name;
        clone_norm_name = norm_name;
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
      std::string original_format = original["format"].str();
      parser::FileFormat original_file_format;
      if (!parser::FileFormatBuilder::build(original_format, original_file_format, msg)) {
        return false;
      }

      mongo::BSONArray clone_format_array;
      std::string clone_format_string;
      if (format != original_file_format) {
        clone_format_array = format.to_bson();
        std::vector< mongo::BSONElement > original_columns = original["columns"].Array();

        mongo::BSONObjIterator clone_columns_it = clone_format_array.begin();

        while (clone_columns_it.more()) {

          mongo::BSONObj clone_column = clone_columns_it.next().Obj();
          bool found = false;
          for (std::vector< mongo::BSONElement>::iterator original_it = original_columns.begin();
               original_it != original_columns.end();
               original_it++) {

            if (clone_column["name"] == original_it->Obj()["name"]) {
              found = true;
            }
          }
          if (!found) {
            msg = "Column " + clone_column["name"].str() +  " not found in the original dataset";
            return false;
          }
        }

        clone_format_string = format.format();

      } else {
        std::vector< mongo::BSONElement > columns = original["columns"].Array();

        mongo::BSONArrayBuilder ab;
        for (std::vector< mongo::BSONElement >::iterator it = columns.begin(); it < columns.end(); it++) {
          ab.append(*it);
        }
        clone_format_array = ab.arr();
        clone_format_string = original["format"].str();
      }

      // Check if the extra_metadata should be replaced
      mongo::BSONObj extra_metadata_obj;
      if (!extra_metadata.empty()) {
        extra_metadata_obj = datatypes::extra_metadata_to_bson(extra_metadata);
      } else {
        extra_metadata_obj = original["extra_metadata"].Obj();
      }

      if (dataset_id[0] == 'a') {
        int a_id;
        if (!helpers::get_counter("annotations", a_id, msg))  {
          return false;
        }
        _id = "a" + utils::integer_to_string(a_id);
      } else {
        int e_id;
        if (!helpers::get_counter("experiments", e_id, msg))  {
          return false;
        }
        _id = "e" + utils::integer_to_string(e_id);
      }

      mongo::BSONObjBuilder cloned_builder;

      mongo::BSONObj::iterator i = original.begin();
      while ( i.more() ) {
        mongo::BSONElement e = i.next();
        if (e.fieldName() == std::string("_id")) {
          cloned_builder.append("_id", _id);
        } else if (e.fieldName() == std::string("name")) {
          cloned_builder.append("name", clone_name);
        } else if (e.fieldName() == std::string("norm_name")) {
          cloned_builder.append("norm_name", clone_norm_name);
        } else if (e.fieldName() == std::string("description")) {
          cloned_builder.append("description", clone_description);
        } else if (e.fieldName() == std::string("norm_description")) {
          cloned_builder.append("norm_description", clone_norm_description);
        }  else if (e.fieldName() == std::string("format")) {
          cloned_builder.append("format", clone_format_string);
        } else if (e.fieldName() == std::string("columns")) {
          cloned_builder.append("columns", clone_format_array);
        } else if (e.fieldName() == std::string("extra_metadata")) {
          cloned_builder.append("extra_metadata", extra_metadata_obj);
        } else if (e.fieldName() == std::string("upload_info")) {
          // Inserting upload_info latter
        } else {
          cloned_builder.append(e);
        }
      }

      mongo::BSONObj cloned_metadata  = cloned_builder.obj();

      if (!search::insert_full_text(collection, _id, cloned_metadata, msg)) {
        c.done();
        return false;
      }

      mongo::BSONObjBuilder clone_final_builder;
      clone_final_builder.appendElements(cloned_metadata);

      mongo::BSONObjBuilder upload_info_builder;
      upload_info_builder.append("cloned_from", dataset_id);
      upload_info_builder.append("done", true);
      upload_info_builder.append("upload_end", mongo::jsTime());

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
