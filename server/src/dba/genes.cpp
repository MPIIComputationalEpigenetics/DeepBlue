//
//  genes.cpp
//  epidb
//
//  Created by Felipe Albrecht on 09.09.2015
//  Copyright (c) 2015 Max Planck Institute for Computer Science. All rights reserved.
//


#include <string>

#include <mongo/bson/bson.h>

#include "../connection/connection.hpp"

#include "../datatypes/metadata.hpp"

#include "../parser/gtf.hpp"

#include "../extras/utils.hpp"

#include "annotations.hpp"
#include "collections.hpp"
#include "full_text.hpp"
#include "helpers.hpp"
#include "remove.hpp"
#include "users.hpp"

namespace epidb {
  namespace dba {
    namespace gene_set {

      bool build_upload_info(const std::string &user_key, const std::string &client_address, const std::string &content_format,
                             mongo::BSONObj &upload_info, std::string &msg)
      {
        utils::IdName user;
        if (!users::get_user(user_key, user, msg)) {
          return false;
        }

        mongo::BSONObjBuilder upload_info_builder;

        upload_info_builder.append("user", user.id);
        upload_info_builder.append("content_format", content_format);
        upload_info_builder.append("done", false);
        upload_info_builder.append("client_address", client_address);
        time_t time_;
        time(&time_);
        upload_info_builder.appendTimeT("upload_start", time_);

        upload_info = upload_info_builder.obj();

        return true;
      }

      bool build_metadata(const std::string &name, const std::string &norm_name,
                          const std::string &description, const std::string &norm_description,
                          const std::string &format,
                          const mongo::BSONObj& extra_metadata_obj,
                          const std::string &user_key, const std::string &ip,
                          std::string &gene_set_id,
                          mongo::BSONObj &gene_set_metadata,
                          std::string &msg)
      {
        return true;
      }

      bool update_upload_info(const std::string &collection, const std::string &annotation_id,
                              const size_t total_size, const size_t total_genes, std::string &msg)
      {
        Connection c;
        c->update(helpers::collection_name(collection), BSON("_id" << annotation_id),
                  BSON("$set" << BSON("upload_info.total_genes" << (long long) total_genes << "upload_info.total_size" << (long long) total_size << "upload_info.done" << true << "upload_info.upload_end" << mongo::DATENOW)), false, true);

        if (!c->getLastError().empty()) {
          msg = c->getLastError();
          c.done();
          return false;
        }
        c.done();

        return true;
      }

      mongo::BSONObj to_bson(const parser::GTFRow& row)
      {
        mongo::BSONObjBuilder bob;


        return bob.obj();
      }


      bool insert(const std::string &name, const std::string &norm_name,
                  const std::string &description, const std::string &norm_description,
                  datatypes::Metadata extra_metadata,
                  const parser::GTFPtr &gtf,
                  const std::string &user_key, const std::string &ip,
                  std::string &gene_set_id, std::string &msg)
      {
        mongo::BSONObj gene_set_metadata;
        mongo::BSONObj extra_metadata_obj = datatypes::extra_metadata_to_bson(extra_metadata);

        if (!build_metadata(name, norm_name, description, norm_description, "GTF", extra_metadata_obj,
                            user_key, ip, gene_set_id, gene_set_metadata, msg)) {
          return false;
        }

        mongo::BSONObj upload_info;
        if (!build_upload_info(user_key, ip, "signal", upload_info, msg)) {
          return false;
        }
        mongo::BSONObjBuilder gene_set_builder;
        gene_set_builder.appendElements(gene_set_metadata);
        gene_set_builder.append("upload_info", upload_info);

        mongo::BSONObj e = gene_set_builder.obj();
        Connection c;
        c->insert(helpers::collection_name(Collections::GENE_SETS()), e);
        if (!c->getLastError().empty()) {
          msg = c->getLastError();
          c.done();
          return false;
        }

        if (!search::insert_full_text(Collections::GENE_SETS(), gene_set_id, gene_set_metadata, msg)) {
          c.done();
          std::string new_msg;
          if (!remove::gene_set(gene_set_id, user_key, new_msg)) {
            msg = msg + " " + new_msg;
          }
          return false;
        }

        size_t total_size = 0;
        size_t total_genes = 0;

        for (const auto& row :  gtf->rows()) {

          mongo::BSONObj row_obj = to_bson(row);

          // row to bson
          /*
          mongo::BSONObj r = region_builder.obj();
          std::string collection = helpers::region_collection_name(genome, internal_chromosome);

          if (prev_collection != collection) {
            if (!prev_collection.empty() && bulk.size() > 0) {
              c->insert(prev_collection, bulk);
              if (!c->getLastError().empty()) {
                msg = c->getLastError();
                c.done();
                std::string new_msg;
                if (!remove::experiment(experiment_id, user_key, new_msg)) {
                  msg = msg + " " + new_msg;
                }
                return false;
              }
              bulk.clear();
            }
            prev_collection = collection;
            actual_size = 0;
          }

          total_size += r.objsize();
          bulk.push_back(r);

          if (bulk.size() % BULK_SIZE == 0 || actual_size > MAXIMUM_SIZE) {
            c->insert(collection, bulk);
            if (!c->getLastError().empty()) {
              msg = c->getLastError();
              c.done();
              std::string new_msg;
              if (!remove::experiment(experiment_id, user_key, new_msg)) {
                msg = msg + " " + new_msg;
              }
              return false;
            }
            bulk.clear();
            actual_size = 0;
          }
          */
          total_genes++;
        }

        if (!update_upload_info(Collections::GENE_SETS(), gene_set_id, total_size, total_genes, msg)) {
          std::string new_msg;
          if (!remove::gene_set(gene_set_id, user_key, new_msg)) {
            msg = msg + " " + new_msg;
          }
          return false;
        }

        c.done();
        return true;
      }

    }
  }
}