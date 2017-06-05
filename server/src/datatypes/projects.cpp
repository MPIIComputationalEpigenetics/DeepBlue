//
//  projects.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 27.05.14.
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

#include <mongo/bson/bson.h>

#include "../config/config.hpp"

#include "../connection/connection.hpp"

#include "../dba/collections.hpp"
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
        if (!dba::helpers::get_one(dba::Collections::PROJECTS(), BSON("norm_name" << norm_name), obj)) {
          msg = Error::m(ERR_INVALID_PROJECT, project);
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
        bool result = c->runCommand(config::DATABASE_NAME(), o, info);
        if (!result) {
          // TODO: get info error
          msg = "error setting the project id '" + project_id + "' public.";
          c.done();
          return  false;
        }
        c.done();
        return dba::helpers::notify_change_occurred(dba::Collections::PROJECTS(), msg);;
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

        return dba::helpers::notify_change_occurred(dba::Collections::PROJECTS(), msg);
      }
    }
  }
}
