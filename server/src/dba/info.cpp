//
//  info.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 04.04.14.
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

#include <map>
#include <string>
#include <vector>

#include <mongo/bson/bson.h>

#include "../datatypes/metadata.hpp"

#include "../extras/utils.hpp"

#include "column_types.hpp"
#include "data.hpp"
#include "users.hpp"

namespace epidb {
  namespace dba {
    namespace info {

      const static std::string NORM_("norm_");

      bool get_sample_by_id(const std::string &id, datatypes::Metadata &res, std::string &msg, bool full = false)
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

      bool get_genome(const std::string &id, datatypes::Metadata &res, mongo::BSONObj& chromosomes, std::string &msg, bool full = false)
      {
        mongo::BSONObj result;
        if (!data::genome(id, result, msg))  {
          return false;
        }

        for (mongo::BSONObj::iterator it = result.begin(); it.more(); ) {
          mongo::BSONElement e = it.next();
          if (e.fieldName() == std::string("chromosomes")) {
            chromosomes = e.Obj().getOwned();
          } else  if (full || (strncmp("norm_", e.fieldName(), 5) != 0)) {
            res[e.fieldName()] = utils::bson_to_string(e);
          }
        }
        return true;
      }

      bool get_project(const std::string &id, const std::vector<std::string>& user_projects, std::map<std::string, std::string> &res, std::string &msg, bool full = false)
      {
        mongo::BSONObj result;
        if (!data::project(id, user_projects, result, msg))  {
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

        /*
        std::vector<utils::IdName> syns;
        std::string norm_biosource_name = result["norm_name"].String();

        if (!get_biosource_synonyms(biosource_name, norm_biosource_name, true, "", syns, msg)) {
          return false;
        }
        for(const utils::IdName & id_name: syns) {
          synonyms.push_back(id_name.name);
        }
        */

        return true;
      }

      bool get_epigenetic_mark(const std::string &id, std::map<std::string, std::string> &res,
                               std::map<std::string, std::string> &metadata,
                               std::string &msg, bool full = false)
      {
        mongo::BSONObj result;
        if (!data::epigenetic_mark(id, result, msg))  {
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

      bool get_experiment(const std::string &id, const std::vector<std::string>& user_projects,
                          std::map<std::string, std::string> &metadata,
                          std::map<std::string, std::string> &extra_metadata,
                          std::map<std::string, std::string> &sample_info,
                          std::vector<std::map<std::string, std::string> > &columns,
                          std::map<std::string, std::string> &upload_info,
                          std::string &msg, bool full = false)
      {
        mongo::BSONObj result;
        if (!data::experiment(id, user_projects, result, msg))  {
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
                  metadata["data_type"] = type;
                } else {
                  upload_info[field_name] = utils::bson_to_string(ee);
                }
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

      bool get_experiment_set_info(const std::string& id,  mongo::BSONObj& obj_metadata, std::string& msg)
      {
        mongo::BSONObj data_obj;
        if (!data::experiment_set(id, data_obj, msg)) {
          return false;
        }

        mongo::BSONObjBuilder bob;

        bob.append(data_obj["_id"]);
        bob.append(data_obj["name"]);
        bob.append(data_obj["description"]);
        bob.append(data_obj["public"]);
        bob.append(data_obj["experiments"]);

        obj_metadata = bob.obj();

        return true;
      }

      bool get_query(const std::string &id, mongo::BSONObj &res, std::string &msg)
      {
        mongo::BSONObj result;
        if (!data::query(id, result, msg))  {
          return false;
        }


        mongo::BSONObjBuilder bob;
        std::cerr << result.toString() << std::endl;

        bob.append(result["_id"]);
        bob.append(result["type"]);


        const std::string user_id = result["user"].String();
        std::string user_name;
        if (!dba::users::get_user_name_by_id(user_id, user_name, msg)) {
          return false;
        }
        bob.append("user", user_name);

        mongo::BSONObjBuilder arg_builder;
        auto it = result.getField("args").Obj().begin();
        while (it.more()) {
          mongo::BSONElement e = it.next();
          std::string fieldName = std::string(e.fieldName());
          if (fieldName.compare(0, 5, "norm_") != 0) {
            arg_builder.append(e);
          }
        }

        bob.append("args", arg_builder.obj());
        res = bob.obj();

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

      bool id_to_name(std::map<std::string, std::string> &map, std::string &msg)
      {
        std::string user_name;

        std::string user = map["user"];
        // XXX : Check because version <= 0.9.35 stores the user name, not the ID.
        if (utils::is_id(user, "u")) {
          if (!dba::users::get_user_name_by_id(user, user_name, msg)) {
            return false;
          }
          map["user"] = user_name;
        }

        return true;
      }
    }
  }
}