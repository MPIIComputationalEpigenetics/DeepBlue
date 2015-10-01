//
//  list.cpp
//  epidb
//
//  Created by Felipe Albrecht on 07.04.14.
//  Copyright (c) 2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include <string>
#include <vector>

#include <mongo/bson/bson.h>

#include "collections.hpp"
#include "column_types.hpp"
#include "config.hpp"
#include "dba.hpp"
#include "helpers.hpp"
#include "list.hpp"

#include "../algorithms/levenshtein.hpp"

#include "../connection/connection.hpp"

#include "../datatypes/metadata.hpp"
#include "../datatypes/user.hpp"

#include "../dba/users.hpp"

#include "../extras/utils.hpp"

namespace epidb {
  namespace dba {
    namespace list {

      bool users(const std::string &user_key, std::vector<utils::IdName> &result, std::string &msg)
      {
        return helpers::get(Collections::USERS(), result, msg);
      }

      bool genomes(const std::string &user_key, std::vector<utils::IdName> &result, std::string &msg)
      {
        return helpers::get(Collections::GENOMES(), result, msg);
      }

      bool biosources(const datatypes::Metadata &metadata,
                      std::vector<utils::IdName> &result, std::string &msg)
      {
        mongo::BSONObjBuilder query_builder;
        datatypes::Metadata::const_iterator it;
        for (const auto& p : metadata) {
          query_builder.append("extra_metadata." + p.first, p.second);
        }
        return helpers::get(Collections::BIOSOURCES(), query_builder.obj(), result, msg);
      }

      bool techniques(const std::string &user_key, std::vector<utils::IdName> &result, std::string &msg)
      {
        return helpers::get(Collections::TECHNIQUES(), result, msg);
      }

      bool samples(const std::string &user_key, const mongo::BSONArray &biosources, const datatypes::Metadata &metadata,
                   std::vector<std::string> &result, std::string &msg)
      {
        mongo::BSONObjBuilder query_builder;

        if (!biosources.isEmpty()) {
          query_builder.append("norm_biosource_name", BSON("$in" << biosources));
        }

        datatypes::Metadata::const_iterator it;
        for (it = metadata.begin(); it != metadata.end(); ++it) {
          query_builder.append(it->first, it->second);
        }

        std::vector<mongo::BSONObj> samples;
        std::vector<std::string> fields;
        mongo::BSONObj query = query_builder.obj();
        if (!helpers::get(Collections::SAMPLES(), query, fields, samples, msg)) {
          return false;
        }

        for (const mongo::BSONObj & o : samples) {
          result.push_back(o["_id"].String());
        }

        return true;
      }

      /*
      * \brief List all projects that MUST NOT be  available to the user
      */
      bool private_projects(const std::string &user_key, std::vector<utils::IdName> &result, std::string &msg)
      {
        mongo::BSONObj user_bson;
        if (!helpers::get_one(Collections::USERS(), BSON("key" << user_key), user_bson, msg)) {
          return false;
        }

        bool user_admin = false;
        if (user_bson.hasField("admin")) {
          user_admin = user_bson["admin"].Bool();
        }

        if (user_admin) {
          result.clear();
          return true;
        }

        mongo::BSONObj private_projects = BSON("public" << false);
        mongo::BSONObj full_query;

        if (user_bson.hasField("projects")) {
          std::vector<std::string> ps;
          std::vector<mongo::BSONElement> projects = user_bson["projects"].Array();
          for (const auto& p : projects) {
            ps.push_back(p.String());
          }

          mongo::BSONObj user_projects = BSON("_id" << BSON("$nin" << helpers::build_array(ps)));
          full_query = BSON("$and" << BSON_ARRAY(private_projects << user_projects));
        } else {
          full_query = private_projects;
        }

        std::vector<mongo::BSONObj> projects;
        if (!helpers::get(Collections::PROJECTS(), full_query, projects, msg)) {
          return false;
        }

        result = helpers::bsons_to_id_names(projects);

        return true;
      }

      /*
      * \brief List all projects that are available to the user
      */
      bool projects(const std::string &user_key, std::vector<utils::IdName> &result, std::string &msg)
      {
        mongo::BSONObj user_bson;
        if (!helpers::get_one(Collections::USERS(), BSON("key" << user_key), user_bson, msg)) {
          return false;
        }

        datatypes::User user;
        if (!dba::users::get_user_by_key(user_key, user, msg)) {
          return false;
        }
        mongo::BSONObj full_query;

        // list all project if is admin
        if (!user.is_admin()) {
          mongo::BSONObj public_projects = BSON("public" << true);
          std::vector<std::string> ps;

          if (user_bson.hasField("projects")) {
            std::vector<mongo::BSONElement> projects = user_bson["projects"].Array();
            for (const auto& p : projects) {
              ps.push_back(p.String());
            }
          }
          full_query = BSON("$or" << BSON_ARRAY(public_projects << BSON("_id" << BSON("$in" << helpers::build_array(ps)))));
        }

        std::vector<mongo::BSONObj> projects;
        if (!helpers::get(Collections::PROJECTS(), full_query, projects, msg)) {
          return false;
        }

        result = helpers::bsons_to_id_names(projects);

        return true;
      }

      bool epigenetic_marks(const std::string &user_key, std::vector<utils::IdName> &result, std::string &msg)
      {
        return helpers::get(Collections::EPIGENETIC_MARKS(), result, msg);
      }

      bool experiments(const mongo::BSONObj query, std::vector<utils::IdName> &result, std::string &msg)
      {
        return experiments(mongo::Query(query), result, msg);
      }

      bool experiments(const mongo::Query query, std::vector<utils::IdName> &result, std::string &msg)
      {
        std::vector<std::string> fields;
        fields.push_back("_id");
        fields.push_back("name");

        std::vector<mongo::BSONObj> objects;
        if (!helpers::get(Collections::EXPERIMENTS(), query, fields, objects, msg)) {
          return false;
        }

        for (const mongo::BSONObj & o : objects) {
          utils::IdName id_name(o["_id"].String(), o["name"].String());
          result.push_back(id_name);
        }

        return true;
      }

      bool annotations(const std::string &genome, const std::string &user_key, std::vector<utils::IdName> &result, std::string &msg)
      {
        std::vector<mongo::BSONObj> objects;
        if (!helpers::get(Collections::ANNOTATIONS(), "norm_genome", genome, objects, msg)) {
          return false;
        }

        for (const mongo::BSONObj & o : objects) {
          utils::IdName names(o["_id"].String(), o["name"].String());
          result.push_back(names);
        }

        return true;
      }

      bool annotations(const std::string &user_key, std::vector<utils::IdName> &result, std::string &msg)
      {
        return helpers::get(Collections::ANNOTATIONS(), result, msg);
      }

      bool column_types(const std::string &user_key, std::vector<utils::IdName> &content, std::string  &msg)
      {
        return dba::columns::list_column_types(user_key, content, msg);
      }

      //-----

      bool column_types(const std::string &user_key, std::vector<std::string> &content, std::string  &msg)
      {
        Connection c;
        std::auto_ptr<mongo::DBClientCursor> data_cursor = c->query(helpers::collection_name(Collections::COLUMN_TYPES()), mongo::BSONObj());

        processing::StatusPtr status = processing::build_dummy_status();

        while (data_cursor->more()) {
          mongo::BSONObj o = data_cursor->next().getOwned();
          columns::ColumnTypePtr column_type;
          if (! epidb::dba::columns::column_type_bsonobj_to_class(o, status, column_type, msg))  {
            return false;
          }
          content.push_back(column_type->str());
        }

        c.done();
        return true;
      }

      bool similar_biosources(const std::string &name, const std::string &user_key,
                              std::vector<utils::IdName> &result, std::string &msg)
      {
        return similar(Collections::BIOSOURCES(), utils::normalize_name(name), user_key, result, msg);
      }

      bool similar_techniques(const std::string &name, const std::string &user_key,
                              std::vector<utils::IdName> &result, std::string &msg)
      {
        return similar(Collections::TECHNIQUES(), utils::normalize_name(name), user_key, result, msg);
      }

      bool similar_projects(const std::string &name, const std::string &user_key,
                            std::vector<utils::IdName> &result, std::string &msg)
      {
        return similar(Collections::PROJECTS(), utils::normalize_name(name), user_key, result, msg);
      }

      bool similar_epigenetic_marks(const std::string &name, const std::string &user_key,
                                    std::vector<utils::IdName> &result, std::string &msg)
      {
        return similar(Collections::EPIGENETIC_MARKS(),  utils::normalize_epigenetic_mark(name), user_key, result, msg);
      }

      bool similar_genomes(const std::string &name, const std::string &user_key,
                           std::vector<utils::IdName> &result, std::string &msg)
      {
        return similar(Collections::GENOMES(), utils::normalize_epigenetic_mark(name), user_key, result, msg);
      }

      bool similar_experiments(const std::string &name, const std::string &genome, const std::string &user_key,
                               std::vector<utils::IdName> &result, std::string &msg)
      {
        return similar(Collections::EXPERIMENTS(), "name", name, "genome", genome, user_key, result, msg);
      }

      bool similar(const std::string &where, const std::string &what,
                   const std::string &user_key, std::vector<utils::IdName> &result, std::string &msg,
                   const size_t total)
      {
        std::vector<utils::IdName> id_names;
        if (!helpers::get(where, id_names, msg)) {
          return false;
        }

        std::vector<std::string> names;
        std::map<std::string, std::string> id_name_map;
        for (const utils::IdName & id_name : id_names) {
          id_name_map[id_name.name] = id_name.id;
          names.push_back(id_name.name);
        }
        std::vector<std::string> ordered = epidb::algorithms::Levenshtein::order_by_score(what, names);

        size_t count = 0;
        for (const std::string & name : ordered) {
          utils::IdName id_name(id_name_map[name], name);
          result.push_back(id_name);
          count++;
          if (count >= total) {
            break;
          }
        }

        return true;
      }

      bool similar(const std::string &where, const std::string &field, const std::string &what,
                   const std::string &filter_field, const std::string &filter_what,
                   const std::string &user_key, std::vector<utils::IdName> &result, std::string &msg,
                   const size_t total)
      {
        std::vector<mongo::BSONObj> docs;

        if (!helpers::get(where, filter_field, filter_what, docs, msg)) {
          return false;
        }

        std::vector<std::string> names;
        std::map<std::string, std::string> id_name_map;
        for (std::vector<mongo::BSONObj>::iterator it = docs.begin(); it != docs.end(); ++it) {
          std::string field_name(it->getField(field).str());
          names.push_back(field_name);
          id_name_map[field_name] = it->getField("_id").String();
        }

        std::vector<std::string> ordered = epidb::algorithms::Levenshtein::order_by_score(what, names);

        size_t count = 0;
        for (const std::string & name : ordered) {
          utils::IdName id_name(id_name_map[name], name);
          result.push_back(id_name);
          count++;
          if (count >= total) {
            break;
          }
        }

        return true;
      }

      bool in_use(const std::string &collection, const std::string &key_name, const std::string &user_key,
                  std::vector<utils::IdNameCount> &names, std::string &msg)
      {
        std::vector<utils::IdName> user_projects;
        if (!projects(user_key, user_projects, msg)) {
          return false;
        }

        std::vector<std::string> project_names;
        for (const auto& project : user_projects) {
          project_names.push_back(project.name);
        }

        mongo::BSONArray projects_array = helpers::build_array(project_names);

        // Select experiments that are uploaded and from πublic projects or that the user has permission
        mongo::BSONObj done = BSON("upload_info.done" << true);
        mongo::BSONObj user_projects_bson = BSON("project" << BSON("$in" << projects_array));
        mongo::BSONObj query = BSON("$and" << BSON_ARRAY(done <<  user_projects_bson));
        mongo::BSONObj match = BSON("$match" << query);

        // Group by count
        mongo::BSONObj group = BSON( "$group" << BSON( "_id" << key_name << "total" << BSON( "$sum" << 1 ) ) );

        mongo::BSONArray pipeline = BSON_ARRAY( match << group );

        mongo::BSONObj agg_command = BSON( "aggregate" << Collections::EXPERIMENTS() << "pipeline" << pipeline);

        Connection c;

        mongo::BSONObj res;
        c->runCommand(config::DATABASE_NAME(), agg_command, res);

        if (!res.getField("ok").trueValue()) {
          msg = res.getStringField("errmsg");
          c.done();
          return false;
        }

        std::vector<mongo::BSONElement> result = res["result"].Array();

        for (const mongo::BSONElement & be : result) {
          std::string norm_name = utils::normalize_name(be["_id"].String());
          long count = be["total"].safeNumberLong();

          utils::IdNameCount inc;
          if (collection == Collections::SAMPLES()) {
            inc = utils::IdNameCount(norm_name, "", count);
          } else {
            utils::IdName id_name;
            if (!helpers::get_name(collection, norm_name, id_name, msg)) {
              c.done();
              return false;
            }
            inc = utils::IdNameCount(id_name.id, id_name.name, count);
          }
          names.push_back(inc);
        }

        c.done();
        return true;
      }
    }
  }
}
