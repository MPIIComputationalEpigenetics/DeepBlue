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
          result = data_cursor->next().getOwned();
        } else {
          msg = "Sample id '" + id + "' not found.";
          c.done();
          return false;
        }
        c.done();

        for (mongo::BSONObj::iterator it = result.begin(); it.more(); ) {
          mongo::BSONElement e = it.next();
          if (full || (strncmp("norm_", e.fieldName(), 5) != 0)) {
            res[e.fieldName()] = e.str();
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
          result = data_cursor->next().getOwned();
        } else {
          msg = "genome with id " + id + " not found.";
          c.done();
          return false;
        }
        c.done();

        for (mongo::BSONObj::iterator it = result.begin(); it.more(); ) {
          mongo::BSONElement e = it.next();
          if (full || (strncmp("norm_", e.fieldName(), 5) != 0)) {
            res[e.fieldName()] = e.str();
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
          result = data_cursor->next().getOwned();
        } else {
          msg = "project with id " + id + " not found.";
          c.done();
          return false;
        }
        c.done();

        for (mongo::BSONObj::iterator it = result.begin(); it.more(); ) {
          mongo::BSONElement e = it.next();
          if (full || (strncmp("norm_", e.fieldName(), 5) != 0)) {
            res[e.fieldName()] = e.str();
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
              metadata[ee.fieldName()] = ee.str();
            }
          } else if (full || (strncmp("norm_", e.fieldName(), 5) != 0)) {
            res[e.fieldName()] = e.str();
          }
        }
        return true;
      }

      bool get_bio_source(const std::string &id, std::map<std::string, std::string> &res,
                          std::map<std::string, std::string> &metadata, std::string &msg, bool full = false)
      {
        mongo::ScopedDbConnection c(config::get_mongodb_server());

        mongo::BSONObj query = BSON("_id" << id);
        std::auto_ptr<mongo::DBClientCursor> data_cursor = c->query(helpers::collection_name(Collections::BIO_SOURCES()), query, 1);
        mongo::BSONObj result;
        if (data_cursor->more()) {
          result = data_cursor->next().getOwned();
        } else {
          msg = "bio source with id " + id + " not found.";
          c.done();
          return false;
        }
        c.done();

        for (mongo::BSONObj::iterator it = result.begin(); it.more(); ) {
          mongo::BSONElement e = it.next();
          if (std::string(e.fieldName()) == "extra_metadata") {
            for (mongo::BSONObj::iterator itt = e.Obj().begin(); itt.more(); ) {
              mongo::BSONElement ee = itt.next();
              metadata[ee.fieldName()] = ee.str();
            }
          } else if (full || (strncmp("norm_", e.fieldName(), 5) != 0)) {
            res[e.fieldName()] = e.str();
          }
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
          result = data_cursor->next().getOwned();
        } else {
          msg = "epigenetic mark with id " + id + " not found.";
          c.done();
          return false;
        }
        c.done();

        for (mongo::BSONObj::iterator it = result.begin(); it.more(); ) {
          mongo::BSONElement e = it.next();
          if (full || (strncmp("norm_", e.fieldName(), 5) != 0)) {
            res[e.fieldName()] = e.str();
          }
        }
        return true;
      }

      bool get_annotation(const std::string &id, std::map<std::string, std::string> &res,
                          std::map<std::string, std::string> &metadata, std::string &msg, bool full = false)
      {
        mongo::ScopedDbConnection c(config::get_mongodb_server());

        mongo::BSONObj query = BSON("_id" << id);
        std::auto_ptr<mongo::DBClientCursor> data_cursor = c->query(helpers::collection_name(Collections::ANNOTATIONS()), query, 1);
        mongo::BSONObj result;
        if (data_cursor->more()) {
          result = data_cursor->next().getOwned();
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
              metadata[ee.fieldName()] = ee.str();
            }
          } else if (full || (strncmp("norm_", e.fieldName(), 5) != 0)) {
            res[e.fieldName()] = e.str();
          }
        }
        return true;
      }

      bool get_experiment(const std::string &id, std::map<std::string, std::string> &res,
                          std::map<std::string, std::string> &metadata, std::string &msg, bool full = false)
      {
        mongo::ScopedDbConnection c(config::get_mongodb_server());

        mongo::BSONObj query = BSON("_id" << id);
        std::auto_ptr<mongo::DBClientCursor> data_cursor = c->query(helpers::collection_name(Collections::EXPERIMENTS()), query, 1);
        mongo::BSONObj result;
        if (data_cursor->more()) {
          result = data_cursor->next().getOwned();
        } else {
          msg = "experiment with id " + id + " not found.";
          c.done();
          return false;
        }
        c.done();

        for (mongo::BSONObj::iterator it = result.begin(); it.more(); ) {
          mongo::BSONElement e = it.next();
          if (std::string(e.fieldName()) == "extra_metadata") {
            for (mongo::BSONObj::iterator itt = e.Obj().begin(); itt.more(); ) {
              mongo::BSONElement ee = itt.next();
              metadata[ee.fieldName()] = ee.str();
            }
          } else if (full || (strncmp("norm_", e.fieldName(), 5) != 0)) {
            res[e.fieldName()] = e.str();
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
          result = data_cursor->next().getOwned();
        } else {
          msg = "query with id " + id + " not found.";
          c.done();
          return false;
        }
        c.done();

        res["genome"] = result.getField("genome").str();
        res["_id"] = result.getField("_id").str();
        res["user"] = result.getField("user").str();
        res["type"] = result.getField("type").str();

        mongo::BSONObjBuilder arg_builder;
        mongo::BSONObj args = result.getField("args").Obj();
        for (mongo::BSONObj::iterator it = args.begin(); it.more();) {
          mongo::BSONElement e = it.next();
          std::string fieldName = std::string(e.fieldName());
          if (fieldName.find("start") == 0) {
            arg_builder.appendNumber("start", e.Long());
          } else if (fieldName.find("end") == 0) {
            arg_builder.appendNumber("end", e.Long());
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
          result = data_cursor->next().getOwned();
        } else {
          msg = "Sample Field with id " + id + " not found.";
          c.done();
          return false;
        }
        c.done();

        for (mongo::BSONObj::iterator it = result.begin(); it.more(); ) {
          mongo::BSONElement e = it.next();
          if (full || (strncmp("norm_", e.fieldName(), 5) != 0)) {
            res[e.fieldName()] = e.str();
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
          result = data_cursor->next().getOwned();
        } else {
          msg = "Tiling Regions id " + id + " not found.";
          c.done();
          return false;
        }
        c.done();

        for (mongo::BSONObj::iterator it = result.begin(); it.more(); ) {
          mongo::BSONElement e = it.next();
          if (full || (strncmp("norm_", e.fieldName(), 5) != 0)) {
            res[e.fieldName()] = e.str();
          }
        }
        return true;
      }

    }
  }
}