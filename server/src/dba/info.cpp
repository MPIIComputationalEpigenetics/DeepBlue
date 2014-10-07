//
//  info.cpp
//  epidb
//
//  Created by Felipe Albrecht on 04.04.14.
//  Copyright (c) 2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include <map>
#include <string>
#include <vector>

#include <boost/foreach.hpp>

#include <mongo/bson/bson.h>
#include <mongo/client/dbclient.h>

#include "../engine/commands.hpp"
#include "../extras/utils.hpp"
#include "../parser/genome_data.hpp"

#include "config.hpp"
#include "collections.hpp"
#include "column_types.hpp"
#include "dba.hpp"
#include "helpers.hpp"

#include "../regions.hpp"

namespace epidb {
  namespace dba {
    namespace info {
      bool get_sample_by_id(const std::string &id, std::map<std::string, std::string> &res, std::string &msg, bool full = false)
      {
        mongo::ScopedDbConnection c(config::get_mongodb_server());

        mongo::BSONObj query = BSON("_id" << id);
        std::auto_ptr<mongo::DBClientCursor> data_cursor = c->query(helpers::collection_name(Collections::SAMPLES()), query, 1);
        mongo::BSONObj result;
        if (data_cursor->more()) {
          result = data_cursor->next();
        } else {
          msg = "Sample id '" + id + "' not found.";
          c.done();
          return false;
        }
        c.done();

        for (mongo::BSONObj::iterator it = result.begin(); it.more(); ) {
          mongo::BSONElement e = it.next();
          if (full || (strncmp("norm_", e.fieldName(), 5) != 0)) {
            res[e.fieldName()] = utils::bson_to_string(e);
          }
        }
        return true;
      }

      bool get_genome(const std::string &id, std::map<std::string, std::string> &res, std::string &msg, bool full = false)
      {
        //  TODO: move to a generic function
        mongo::ScopedDbConnection c(config::get_mongodb_server());

        mongo::BSONObj query = BSON("_id" << id);
        std::auto_ptr<mongo::DBClientCursor> data_cursor = c->query(helpers::collection_name(Collections::GENOMES()), query, 1);
        mongo::BSONObj result;
        if (data_cursor->more()) {
          result = data_cursor->next();
        } else {
          msg = "genome with id " + id + " not found.";
          c.done();
          return false;
        }
        c.done();

        for (mongo::BSONObj::iterator it = result.begin(); it.more(); ) {
          mongo::BSONElement e = it.next();
          if (full || (strncmp("norm_", e.fieldName(), 5) != 0)) {
            res[e.fieldName()] = utils::bson_to_string(e);
          }
        }
        return true;
      }

      bool get_project(const std::string &id, std::map<std::string, std::string> &res, std::string &msg, bool full = false)
      {
        //  TODO: move to a generic function
        mongo::ScopedDbConnection c(config::get_mongodb_server());

        mongo::BSONObj query = BSON("_id" << id);
        std::auto_ptr<mongo::DBClientCursor> data_cursor = c->query(helpers::collection_name(Collections::PROJECTS()), query, 1);
        mongo::BSONObj result;
        if (data_cursor->more()) {
          result = data_cursor->next();
        } else {
          msg = "project with id " + id + " not found.";
          c.done();
          return false;
        }
        c.done();

        for (mongo::BSONObj::iterator it = result.begin(); it.more(); ) {
          mongo::BSONElement e = it.next();
          if (full || (strncmp("norm_", e.fieldName(), 5) != 0)) {
            res[e.fieldName()] = utils::bson_to_string(e);
          }
        }
        return true;
      }

      bool get_technique(const std::string &id, std::map<std::string, std::string> &res,
                         std::map<std::string, std::string> &metadata, std::string &msg, bool full = false)
      {
        mongo::ScopedDbConnection c(config::get_mongodb_server());

        mongo::BSONObj query = BSON("_id" << id);
        std::auto_ptr<mongo::DBClientCursor> data_cursor = c->query(helpers::collection_name(Collections::TECHNIQUES()), query, 1);
        mongo::BSONObj result;
        if (data_cursor->more()) {
          result = data_cursor->next().getOwned();
        } else {
          msg = "technique with id " + id + " not found.";
          c.done();
          return false;
        }
        c.done();

        for (mongo::BSONObj::iterator it = result.begin(); it.more(); ) {
          mongo::BSONElement e = it.next();
          if (std::string(e.fieldName()) == "extra_metadata") {
            for (mongo::BSONObj::iterator itt = e.Obj().begin(); itt.more(); ) {
              mongo::BSONElement ee = itt.next();
              metadata[ee.fieldName()] = utils::bson_to_string(ee);
            }
          } else if (full || (strncmp("norm_", e.fieldName(), 5) != 0)) {
            res[e.fieldName()] = utils::bson_to_string(e);
          }
        }
        return true;
      }

      bool get_biosource(const std::string &id, std::map<std::string, std::string> &res,
                         std::map<std::string, std::string> &metadata,
                         std::vector<std::string> &synonyms,
                         std::string &msg,
                         bool full = false)
      {
        mongo::ScopedDbConnection c(config::get_mongodb_server());

        mongo::BSONObj query = BSON("_id" << id);
        std::auto_ptr<mongo::DBClientCursor> data_cursor = c->query(helpers::collection_name(Collections::BIOSOURCES()), query, 1);
        mongo::BSONObj result;
        if (data_cursor->more()) {
          result = data_cursor->next();
        } else {
          msg = "biosource with id " + id + " not found.";
          c.done();
          return false;
        }
        c.done();

        std::string biosource_name = result["name"].String();
        std::string norm_biosource_name = result["norm_name"].String();

        for (mongo::BSONObj::iterator it = result.begin(); it.more(); ) {
          mongo::BSONElement e = it.next();
          if (std::string(e.fieldName()) == "extra_metadata") {
            for (mongo::BSONObj::iterator itt = e.Obj().begin(); itt.more(); ) {
              mongo::BSONElement ee = itt.next();
              metadata[ee.fieldName()] = utils::bson_to_string(ee);
            }
          } else if (full || (strncmp("norm_", e.fieldName(), 5) != 0)) {
            res[e.fieldName()] = utils::bson_to_string(e);
          }
        }

        utils::IdName id_name_biosource(id, biosource_name);
        std::vector<utils::IdName> syns;
        if (!get_biosource_synonyms(biosource_name, norm_biosource_name, true, "", syns, msg)) {
          return false;
        }
        BOOST_FOREACH(const utils::IdName & id_name, syns) {
          synonyms.push_back(id_name.name);
        }

        return true;
      }

      bool get_epigenetic_mark(const std::string &id, std::map<std::string, std::string> &res, std::string &msg, bool full = false)
      {
        //  TODO: move to a generic function
        mongo::ScopedDbConnection c(config::get_mongodb_server());

        mongo::BSONObj query = BSON("_id" << id);
        std::auto_ptr<mongo::DBClientCursor> data_cursor = c->query(helpers::collection_name(Collections::EPIGENETIC_MARKS()), query, 1);
        mongo::BSONObj result;
        if (data_cursor->more()) {
          result = data_cursor->next();
        } else {
          msg = "epigenetic mark with id " + id + " not found.";
          c.done();
          return false;
        }
        c.done();

        for (mongo::BSONObj::iterator it = result.begin(); it.more(); ) {
          mongo::BSONElement e = it.next();
          if (full || (strncmp("norm_", e.fieldName(), 5) != 0)) {
            res[e.fieldName()] = utils::bson_to_string(e);
          }
        }
        return true;
      }

      bool get_annotation(const std::string &id, std::map<std::string,
                          std::string> &metadata,
                          std::map<std::string, std::string> &extra_metadata,
                          std::vector<std::map<std::string, std::string> > &columns,
                          std::map<std::string, std::string> &upload_info,
                          std::string &msg, bool full = false)
      {
        mongo::ScopedDbConnection c(config::get_mongodb_server());

        mongo::BSONObj query = BSON("_id" << id);
        std::auto_ptr<mongo::DBClientCursor> data_cursor = c->query(helpers::collection_name(Collections::ANNOTATIONS()), query, 1);
        mongo::BSONObj result;
        if (data_cursor->more()) {
          result = data_cursor->next();
        } else {
          msg = "annotation with id " + id + " not found.";
          c.done();
          return false;
        }
        c.done();

        for (mongo::BSONObj::iterator it = result.begin(); it.more(); ) {
          mongo::BSONElement e = it.next();
          if (std::string(e.fieldName()) == "extra_metadata") {
            for (mongo::BSONObj::iterator itt = e.Obj().begin(); itt.more(); ) {
              mongo::BSONElement ee = itt.next();
              extra_metadata[ee.fieldName()] = utils::bson_to_string(ee);
            }
          } else if (std::string(e.fieldName()) == "upload_info") {
            for (mongo::BSONObj::iterator itt = e.Obj().begin(); itt.more(); ) {
              mongo::BSONElement ee = itt.next();
              upload_info[ee.fieldName()] = utils::bson_to_string(ee);
            }
          } else if (std::string(e.fieldName()) == "columns") {
            for (mongo::BSONObj::iterator itt = e.Obj().begin(); itt.more(); ) {
              mongo::BSONElement ee = itt.next();
              columns.push_back(columns::dataset_column_to_map(ee.Obj()));
            }
          } else if (full || (strncmp("norm_", e.fieldName(), 5) != 0)) {
            metadata[e.fieldName()] = utils::bson_to_string(e);
          }
        }
        return true;
      }

      bool get_experiment(const std::string &id, std::map<std::string, std::string> &metadata,
                          std::map<std::string, std::string> &extra_metadata,
                          std::map<std::string, std::string> &sample_info,
                          std::vector<std::map<std::string, std::string> > &columns,
                          std::map<std::string, std::string> &upload_info,
                          std::string &msg, bool full = false)
      {
        mongo::ScopedDbConnection c(config::get_mongodb_server());

        mongo::BSONObj query = BSON("_id" << id);
        std::auto_ptr<mongo::DBClientCursor> data_cursor = c->query(helpers::collection_name(Collections::EXPERIMENTS()), query, 1);
        mongo::BSONObj result;
        if (data_cursor->more()) {
          result = data_cursor->next();
        } else {
          msg = "experiment with id " + id + " not found.";
          c.done();
          return false;
        }
        c.done();

        for (mongo::BSONObj::iterator it = result.begin(); it.more(); ) {
          mongo::BSONElement e = it.next();
          if (std::string(e.fieldName()) == "sample_info") {
            for (mongo::BSONObj::iterator itt = e.Obj().begin(); itt.more(); ) {
              mongo::BSONElement ee = itt.next();
              sample_info[ee.fieldName()] = utils::bson_to_string(ee);
            }
          } else if (std::string(e.fieldName()) == "extra_metadata") {
            for (mongo::BSONObj::iterator itt = e.Obj().begin(); itt.more(); ) {
              mongo::BSONElement ee = itt.next();
              extra_metadata[ee.fieldName()] = utils::bson_to_string(ee);
            }
          } else if (std::string(e.fieldName()) == "upload_info") {
            for (mongo::BSONObj::iterator itt = e.Obj().begin(); itt.more(); ) {
              mongo::BSONElement ee = itt.next();
              upload_info[ee.fieldName()] = utils::bson_to_string(ee);
            }
          } else if (std::string(e.fieldName()) == "columns") {
            for (mongo::BSONObj::iterator itt = e.Obj().begin(); itt.more(); ) {
              mongo::BSONElement ee = itt.next();
              columns.push_back(columns::dataset_column_to_map(ee.Obj()));
            }
          } else if (full || (strncmp("norm_", e.fieldName(), 5) != 0)) {
            metadata[e.fieldName()] = utils::bson_to_string(e);
          }
        }
        return true;
      }

      bool get_query(const std::string &id, std::map<std::string, std::string> &res, std::string &msg)
      {
        mongo::ScopedDbConnection c(config::get_mongodb_server());

        mongo::BSONObj query = BSON("_id" << id);
        std::auto_ptr<mongo::DBClientCursor> data_cursor = c->query(helpers::collection_name(Collections::QUERIES()), query, 1);
        if (!c->getLastError().empty()) {
          msg = c->getLastError();
          c.done();
          return false;
        }
        mongo::BSONObj result;
        if (data_cursor->more()) {
          result = data_cursor->next();
        } else {
          msg = "query with id " + id + " not found.";
          c.done();
          return false;
        }
        c.done();

        res["genome"] = utils::bson_to_string(result["genome"]);
        res["_id"] = utils::bson_to_string(result["_id"]);
        res["user"] = utils::bson_to_string(result["user"]);
        res["type"] = utils::bson_to_string(result["type"]);

        mongo::BSONObjBuilder arg_builder;
        mongo::BSONObj args = result.getField("args").Obj();
        for (mongo::BSONObj::iterator it = args.begin(); it.more();) {
          mongo::BSONElement e = it.next();
          std::string fieldName = std::string(e.fieldName());
          if (fieldName.find("start") == 0) {
            arg_builder.appendNumber("start", e.Int());
          } else if (fieldName.find("end") == 0) {
            arg_builder.appendNumber("end", e.Int());
          } else if (fieldName.find("norm_") != 0) {
            arg_builder.append(e);
          }
        }

        res["args"] = arg_builder.obj().jsonString(mongo::Strict, false);
        return true;
      }


      bool get_sample_field(const std::string &id, std::map<std::string, std::string> &res, std::string &msg, bool full = false)
      {
        //  TODO: move to a generic function
        mongo::ScopedDbConnection c(config::get_mongodb_server());

        mongo::BSONObj query = BSON("_id" << id);
        std::auto_ptr<mongo::DBClientCursor> data_cursor = c->query(helpers::collection_name(Collections::SAMPLE_FIELDS()), query, 1);
        mongo::BSONObj result;
        if (data_cursor->more()) {
          result = data_cursor->next();
        } else {
          msg = "Sample Field with id " + id + " not found.";
          c.done();
          return false;
        }
        c.done();

        for (mongo::BSONObj::iterator it = result.begin(); it.more(); ) {
          mongo::BSONElement e = it.next();
          if (full || (strncmp("norm_", e.fieldName(), 5) != 0)) {
            res[e.fieldName()] = utils::bson_to_string(e);
          }
        }
        return true;
      }

      bool get_tiling_region(const std::string &id, std::map<std::string, std::string> &res, std::string &msg, bool full = false)
      {
        //  TODO: move to a generic function
        mongo::ScopedDbConnection c(config::get_mongodb_server());

        mongo::BSONObj query = BSON("_id" << id);
        std::auto_ptr<mongo::DBClientCursor> data_cursor = c->query(helpers::collection_name(Collections::TILINGS()), query, 1);
        mongo::BSONObj result;
        if (data_cursor->more()) {
          result = data_cursor->next();
        } else {
          msg = "Tiling Regions id " + id + " not found.";
          c.done();
          return false;
        }
        c.done();

        for (mongo::BSONObj::iterator it = result.begin(); it.more(); ) {
          mongo::BSONElement e = it.next();
          if (full || (strncmp("norm_", e.fieldName(), 5) != 0)) {
            res[e.fieldName()] = utils::bson_to_string(e);
          }
        }
        return true;
      }

      bool get_column_type(const std::string &id, std::map<std::string, std::string> &res, std::string &msg)
      {
        return columns::get_column_type(id, res, msg);
      }

    }
  }
}