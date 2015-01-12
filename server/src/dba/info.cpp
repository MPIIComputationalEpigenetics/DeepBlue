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

#include "../extras/utils.hpp"

#include "column_types.hpp"
#include "data.hpp"

namespace epidb {
  namespace dba {
    namespace info {

      const static std::string NORM_("norm_");

      bool get_sample_by_id(const std::string &id, std::map<std::string, std::string> &res, std::string &msg, bool full = false)
      {
        mongo::BSONObj result;
        if (!data::sample(id, result, msg))  {
          return false;
        }

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
        mongo::BSONObj result;
        if (!data::genome(id, result, msg))  {
          return false;
        }

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
        mongo::BSONObj result;
        if (!data::project(id, result, msg))  {
          return false;
        }

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
        mongo::BSONObj result;
        if (!data::technique(id, result, msg))  {
          return false;
        }

        for (mongo::BSONObj::iterator it = result.begin(); it.more(); ) {
          mongo::BSONElement e = it.next();
          if (std::string(e.fieldName()) == "extra_metadata") {
            mongo::BSONObj::iterator itt = e.Obj().begin();
            while (itt.more()) {
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
        mongo::BSONObj result;
        if (!data::biosource(id, result, msg))  {
          return false;
        }

        std::string biosource_name = result["name"].String();
        std::string norm_biosource_name = result["norm_name"].String();

        for (mongo::BSONObj::iterator it = result.begin(); it.more(); ) {
          mongo::BSONElement e = it.next();
          if (std::string(e.fieldName()) == "extra_metadata") {
            mongo::BSONObj::iterator itt = e.Obj().begin();
            while (itt.more()) {
              mongo::BSONElement ee = itt.next();
              metadata[ee.fieldName()] = utils::bson_to_string(ee);
            }
          } else if (full || (strncmp("norm_", e.fieldName(), 5) != 0)) {
            res[e.fieldName()] = utils::bson_to_string(e);
          }
        }

        utils::IdName id_name_biosource(id, biosource_name);
        std::vector<utils::IdName> syns;
        /*
        if (!get_biosource_synonyms(biosource_name, norm_biosource_name, true, "", syns, msg)) {
          return false;
        }
        BOOST_FOREACH(const utils::IdName & id_name, syns) {
          synonyms.push_back(id_name.name);
        }
        */

        return true;
      }

      bool get_epigenetic_mark(const std::string &id, std::map<std::string, std::string> &res, std::string &msg, bool full = false)
      {
        mongo::BSONObj result;
        if (!data::epigenetic_mark(id, result, msg))  {
          return false;
        }

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
        mongo::BSONObj result;
        if (!data::annotation(id, result, msg))  {
          return false;
        }

        for (mongo::BSONObj::iterator it = result.begin(); it.more(); ) {
          mongo::BSONElement e = it.next();
          if (std::string(e.fieldName()) == "D") {
            continue;
          }
          if (std::string(e.fieldName()) == "extra_metadata") {
            mongo::BSONObj::iterator itt = e.Obj().begin();
            while (itt.more()) {
              mongo::BSONElement ee = itt.next();
              std::string field_name = ee.fieldName();
              if (field_name.compare(0, 5, NORM_) != 0) {
                extra_metadata[field_name] = utils::bson_to_string(ee);
              }
            }
          } else if (std::string(e.fieldName()) == "upload_info") {
            mongo::BSONObj::iterator itt = e.Obj().begin();
            while (itt.more()) {
              mongo::BSONElement ee = itt.next();
              std::string field_name = ee.fieldName();
              if (field_name.compare(0, 5, NORM_) != 0) {
                upload_info[field_name] = utils::bson_to_string(ee);
              }
            }
          } else if (std::string(e.fieldName()) == "columns") {
            mongo::BSONObj::iterator itt = e.Obj().begin();
            while (itt.more()) {
              mongo::BSONElement ee = itt.next();
              std::string field_name = ee.fieldName();
              if (field_name.compare(0, 5, NORM_) != 0) {
                columns.push_back(columns::dataset_column_to_map(ee.Obj()));
              }
            }
          } else {
            std::string field_name = e.fieldName();
            if (full || field_name.compare(0, 5, "norm_") != 0) {
              metadata[e.fieldName()] = utils::bson_to_string(e);
            }
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
        mongo::BSONObj result;
        if (!data::experiment(id, result, msg))  {
          return false;
        }

        for (mongo::BSONObj::iterator it = result.begin(); it.more(); ) {
          mongo::BSONElement e = it.next();
          if (std::string(e.fieldName()) == "D") {
            continue;
          }
          if (std::string(e.fieldName()) == "sample_info") {
            mongo::BSONObj::iterator itt = e.Obj().begin();
            while (itt.more()) {
              mongo::BSONElement ee = itt.next();
              std::string field_name = ee.fieldName();
              if (field_name.compare(0, 5, "norm_") != 0) {
                sample_info[field_name] = utils::bson_to_string(ee);
              }
            }
          } else if (std::string(e.fieldName()) == "extra_metadata") {
            mongo::BSONObj::iterator itt = e.Obj().begin();
            while (itt.more()) {
              mongo::BSONElement ee = itt.next();
              std::string field_name = ee.fieldName();

              if (field_name.compare(0, 5, "norm_") != 0 &&
                  field_name.compare(0, 2, "__") != 0) {
                extra_metadata[field_name] = utils::bson_to_string(ee);
              }

            }
          } else if (std::string(e.fieldName()) == "upload_info") {
            mongo::BSONObj::iterator itt = e.Obj().begin();
            while (itt.more()) {
              mongo::BSONElement ee = itt.next();
              std::string field_name = ee.fieldName();
              if (field_name.compare(0, 5, "norm_") != 0) {
                if (field_name == "content_format") {
                  std::string type = utils::bson_to_string(ee);
                  if (type == "bed") {
                    metadata["data_type"] = "peaks";
                  } else if ((type == "wig") || (type == "bedgraph")) {
                    metadata["data_type"] = "signal";
                  } else {
                    metadata["data_type"] = "Unknown";
                  }
                }
                upload_info[field_name] = utils::bson_to_string(ee);
              }
            }
          } else if (std::string(e.fieldName()) == "columns") {
            mongo::BSONObj::iterator itt = e.Obj().begin();
            while (itt.more()) {
              mongo::BSONElement ee = itt.next();
              std::string field_name = ee.fieldName();
              if (field_name.compare(0, 5, "norm_") != 0) {
                columns.push_back(columns::dataset_column_to_map(ee.Obj()));
              }
            }
          } else {
            std::string field_name = e.fieldName();
            if (full || (
                  field_name.compare(0, 5, "norm_") != 0 &&
                  field_name.compare(0, 2, "__") != 0)) {
              metadata[e.fieldName()] = utils::bson_to_string(e);
            }
          }
        }
        return true;
      }

      bool get_query(const std::string &id, std::map<std::string, std::string> &res, std::string &msg)
      {
        mongo::BSONObj result;
        if (!data::query(id, result, msg))  {
          return false;
        }

        res["genome"] = utils::bson_to_string(result["genome"]);
        res["_id"] = utils::bson_to_string(result["_id"]);
        res["user"] = utils::bson_to_string(result["user"]);
        res["type"] = utils::bson_to_string(result["type"]);

        mongo::BSONObjBuilder arg_builder;
        mongo::BSONObj args = result.getField("args").Obj();
        mongo::BSONObj::iterator it = args.begin();
        while (it.more()) {
          mongo::BSONElement e = it.next();
          std::string fieldName = std::string(e.fieldName());
          if (fieldName == "start") {
            arg_builder.appendNumber("start", e.Int());
          } else if (fieldName == "end") {
            arg_builder.appendNumber("end", e.Int());
          } else if (fieldName.compare(0, 5, "norm_") != 0) {
            arg_builder.append(e);
          }
        }

        res["args"] = arg_builder.obj().jsonString(mongo::Strict, false);
        return true;
      }

      bool get_tiling_region(const std::string &id, std::map<std::string, std::string> &res, std::string &msg, bool full = false)
      {
        mongo::BSONObj result;
        if (!data::tiling_region(id, result, msg))  {
          return false;
        }

        mongo::BSONObj::iterator it = result.begin();
        while (it.more()) {
          mongo::BSONElement e = it.next();
          std::string field_name = e.fieldName();
          if (full || field_name.compare(0, 5, "norm_") != 0) {
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