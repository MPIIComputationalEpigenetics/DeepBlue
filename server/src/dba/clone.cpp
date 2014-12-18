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

#include "annotations.hpp"
#include "collections.hpp"
#include "config.hpp"
#include "experiments.hpp"
#include "full_text.hpp"
#include "info.hpp"
#include "users.hpp"


#include "../log.hpp"

namespace epidb {
  namespace dba {

    bool clone_dataset(const std::string &dataset_id,
                       const std::string &clone_name, const std::string &norm_clone_name,
                       const std::string &genome, const std::string &norm_genome,
                       const std::string &epigenetic_mark, const std::string &norm_epigenetic_mark,
                       const std::string &sample_id,
                       const std::string &technique, const std::string &norm_technique,
                       const std::string &project, const std::string &norm_project,
                       const std::string &description, const std::string &norm_description,
                       const parser::FileFormat &format, const datatypes::Metadata &extra_metadata,
                       const std::string user_key, const std::string& ip,
                       std::string &_id, std::string &msg)
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
        msg = "Internal Error:  experiment with ID " + dataset_id + " not found.";
        c.done();
        return false;
      }

      std::string clone_genome;
      std::string clone_norm_genome;
      if (genome.empty()) {
        clone_genome = original["genome"].str();
        clone_norm_genome = original["norm_genome"].str();
      } else {
        clone_genome = genome;
        clone_norm_genome = norm_genome;
      }

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
      std::string original_format = original["format"].str();
      parser::FileFormat original_file_format;
      if (!parser::FileFormatBuilder::build(original_format, original_file_format, msg)) {
        c.done();
        return false;
      }


      mongo::BSONArray clone_format_array;
      mongo::BSONArray original_columns_array;
      std::vector< mongo::BSONElement > columns = original["columns"].Array();
      mongo::BSONArrayBuilder ab;
      for (std::vector< mongo::BSONElement >::iterator it = columns.begin(); it < columns.end(); it++) {
        ab.append(*it);
      }
      original_columns_array = ab.arr();

      // Check format
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
            c.done();
            return false;
          }
        }
      }

      // Check if the extra_metadata should be replaced
      mongo::BSONObj extra_metadata_obj;
      if (!extra_metadata.empty()) {
        extra_metadata_obj = datatypes::extra_metadata_to_bson(extra_metadata);
      } else {
        extra_metadata_obj = original["extra_metadata"].Obj();
      }

      DatasetId internal_dataset_id = original[KeyMapper::DATASET()].Int();
      mongo::BSONObj cloned_metadata;


      if (dataset_id[0] == 'a') {
        int a_id;
        if (!helpers::get_counter("annotations", a_id, msg))  {
          c.done();
          return false;
        }
        _id = "a" + utils::integer_to_string(a_id);
        if (!annotations::build_metadata(clone_name, norm_clone_name, clone_genome, clone_norm_genome,
                                   clone_description, clone_norm_description, extra_metadata,
                                   user_key, ip, format,
                                   internal_dataset_id, _id, cloned_metadata, msg)) {
          return false;
        }

      } else {
        int e_id;
        if (!helpers::get_counter("experiments", e_id, msg))  {
          c.done();
          return false;
        }
        _id = "e" + utils::integer_to_string(e_id);
        if (!experiments::build_metadata(clone_name, norm_clone_name, clone_genome, clone_norm_genome,
                                       clone_epigenetic_mark, clone_norm_epigenetic_mark,
                                       clone_sample_id, clone_technique, clone_norm_technique,
                                       clone_project, clone_norm_project,
                                       clone_description, clone_norm_description,
                                       extra_metadata, user_key, ip,
                                       format, internal_dataset_id,
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

      std::string user_name;
      if (!users::get_user_name(user_key, user_name, msg)) {
        c.done();
        return false;
      }

      mongo::BSONObjBuilder upload_info_builder;
      upload_info_builder.append("user", user_name);
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
