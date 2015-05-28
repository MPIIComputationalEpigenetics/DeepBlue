//
//  projects.cpp
//  epidb
//
//  Created by Felipe Albrecht on 27.05.14.
//  Copyright (c) 2013,2014,2015 Max Planck Institute for Computer Science. All rights reserved.
//

#include <string>

#include <mongo/bson/bson.h>

#include "../connection/connection.hpp"

#include "../dba/collections.hpp"
#include "../dba/config.hpp"
#include "../dba/full_text.hpp"
#include "../dba/helpers.hpp"

#include "../extras/utils.hpp"

#include "../errors.hpp"

namespace epidb {
  namespace datatypes {
    namespace projects {


      bool add_project(const std::string &name, const std::string &norm_name,
                       const std::string &description, const std::string &norm_description,
                       const utils::IdName &user,
                       std::string &project_id, std::string &msg)
      {
        {
          int id;
          if (!dba::helpers::get_increment_counter("projects", id, msg) ||
              !dba::helpers::notify_change_occurred(dba::Collections::PROJECTS(), msg)) {
            return false;
          }
          project_id = "p" + utils::integer_to_string(id);
        }
        mongo::BSONObjBuilder search_data_builder;
        search_data_builder.append("_id", project_id);
        search_data_builder.append("name", name);
        search_data_builder.append("norm_name", norm_name);
        search_data_builder.append("description", description);
        search_data_builder.append("norm_description", norm_description);

        mongo::BSONObj search_data = search_data_builder.obj();
        mongo::BSONObjBuilder create_project_builder;
        create_project_builder.appendElements(search_data);

        create_project_builder.append("user", user.id);
        mongo::BSONObj cem = create_project_builder.obj();

        Connection c;
        c->insert(dba::helpers::collection_name(dba::Collections::PROJECTS()), cem);
        if (!c->getLastError().empty()) {
          msg = c->getLastError();
          c.done();
          return false;
        }

        if (!dba::search::insert_full_text(dba::Collections::PROJECTS(), project_id, search_data, msg)) {
          c.done();
          return false;
        }

        c.done();
        return true;
      }

      bool get_id(const std::string &project, std::string& id, std::string &msg)
      {
        if (utils::is_id(project, "p")) {
          id = project;
          return true;
        }

        const std::string norm_name = utils::normalize_name(project);

        mongo::BSONObj obj;
        if (!dba::helpers::get_one(dba::Collections::PROJECTS(), BSON("norm_name" << norm_name), obj, msg)) {
          msg = Error::m(ERR_INVALID_PROJECT_NAME, project.c_str());
          return false;
        }

        id = obj["_id"].str();
        return true;
      }

      bool set_public(const std::string &project_id, const bool set, std::string &msg)
      {
        mongo::BSONObj o = BSON("findandmodify" << dba::Collections::PROJECTS() <<
                                "query" << BSON("_id" << project_id) <<
                                "update" << BSON("$set" << BSON("public" << set)));

        Connection c;
        mongo::BSONObj info;
        bool result = c->runCommand(dba::config::DATABASE_NAME(), o, info);
        if (!result) {
          // TODO: get info error
          msg = "error setting the project id '" + project_id + "' public.";
          c.done();
          return  false;
        }
        c.done();
        return true;
      }

      bool add_user_to_project(const std::string &user_id, const std::string &project_id, const bool include, std::string &msg)
      {
        Connection c;

        mongo::BSONObjBuilder query_builder;
        query_builder.append("_id", user_id);
        mongo::BSONObj query = query_builder.obj();

        mongo::BSONObj append_value;
        if (include) {
          append_value = BSON("$addToSet" << BSON("projects" << project_id));
        } else {
          append_value = BSON("$pull" << BSON("projects" << project_id));
        }

        c->update(dba::helpers::collection_name(dba::Collections::USERS()), query, append_value, true, false);
        if (!c->getLastError().empty()) {
          msg = c->getLastError();
          c.done();
          return false;
        }

        c.done();
        return true;
      }

      bool is_public(const std::string &project_id, bool& ret, std::string &msg)
      {
        if (!dba::helpers::notify_change_occurred(dba::Collections::PROJECTS(), msg)) {
          return false;
        }
        Connection c;

        mongo::BSONObjBuilder query_builder;

        query_builder.append("public", true);
        query_builder.append("_id", project_id);

        mongo::BSONObj query = query_builder.obj();
        long long count = c->count(dba::helpers::collection_name(dba::Collections::PROJECTS()), query);
        if (!c->getLastError().empty()) {
          msg = c->getLastError();
          c.done();
          return false;
        }

        ret = count > 0;
        c.done();
        return true;
      }
    }
  }
}