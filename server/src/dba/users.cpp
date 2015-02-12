//
//  users.cpp
//  epidb
//
//  Created by Felipe Albrecht on 03.11.14.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include <string>

#include <mongo/bson/bson.h>

#include "../connection/connection.hpp"

#include "collections.hpp"
#include "config.hpp"
#include "exists.hpp"
#include "helpers.hpp"

namespace epidb {
  namespace dba {
    namespace users {

      bool is_valid_email(const std::string &email, std::string &msg)
      {
        if (helpers::check_exist(Collections::USERS(), "email", email)) {
          std::stringstream ss;
          ss << "Email '" << email << "' is already in use.";
          msg = ss.str();
          return false;
        }
        return true;
      }

      bool add_user(const std::string &name, const std::string &email, const std::string &institution,
                    const std::string &key, std::string &user_id, std::string &msg)
      {
        {
          int id;
          if (!helpers::get_counter("users", id, msg))  {
            return false;
          }
          user_id = "u" + utils::integer_to_string(id);
        }

        mongo::BSONObjBuilder create_user_builder;
        create_user_builder.append("_id", user_id);
        create_user_builder.append("name", name);
        create_user_builder.append("email", email);
        create_user_builder.append("institution", institution);
        create_user_builder.append("key", key);
        mongo::BSONObj cu = create_user_builder.obj();

        Connection c;
        c->insert(helpers::collection_name(Collections::USERS()), cu);
        if (!c->getLastError().empty()) {
          msg = c->getLastError();
          c.done();
          return false;
        }

        mongo::BSONObjBuilder index_name;
        index_name.append("key", 1);
        c->createIndex(helpers::collection_name(Collections::USERS()), index_name.obj());
        if (!c->getLastError().empty()) {
          msg = c->getLastError();
          c.done();
          return false;
        }

        c.done();
        return true;
      }

      bool get_user_name(const std::string &user_key, std::string &name, std::string &msg)
      {
        utils::IdName id_name;
        if (!helpers::get_name(Collections::USERS(), user_key, id_name, msg)) {
          return false;
        }
        name = id_name.name;
        return true;
      }

      bool get_user_name(const std::string &user_key, utils::IdName &id_name, std::string &msg)
      {
        return helpers::get_name(Collections::USERS(), user_key, id_name, msg);
      }

      bool bind_user(const std::string &email, const std::string &password, const std::string &user_key, const std::string &admin_key, std::string &id, std::string &msg)
      {

        return false;

        /*
        mongo::BSONObjBuilder builder;

        builder.append("_id", _id);
        builder.append("email", email);
        builder.append("password", password);
        builder.append("key", password);

        Connection c;
        c->insert(helpers::collection_name(Collections::WEB_ACCESS()), create_column_type_calculated_builder.obj());

        mongo::BSONObjBuilder index;
        index.append("email", 1);
        index.append("password", 1);
        c->createIndex(helpers::collection_name(Collections::WEB_ACCESS()), index.obj());
        if (!c->getLastError().empty()) {
          msg = c->getLastError();
          c.done();
          return false;
        }
        */
      }

      bool set_user_admin(const std::string &user_id, const bool value, std::string &msg)
      {
        mongo::BSONObj o = BSON("findandmodify" << Collections::USERS() <<
                                "query" << BSON("_id" << user_id) <<
                                "update" << BSON("$set" << BSON("admin" << value)));

        Connection c;
        mongo::BSONObj info;
        bool result = c->runCommand(config::DATABASE_NAME(), o, info);
        if (!result) {
          // TODO: get info error
          msg = "error setting admin in user '" + user_id + "'.";
          c.done();
          return  false;
        }
        c.done();
        return true;
      }

      bool is_admin_key(const std::string &admin_key, bool &ret, std::string &msg)
      {
        Connection c;

        mongo::BSONObjBuilder query_builder;

        query_builder.append("admin", true);
        query_builder.append("key", admin_key);

        mongo::BSONObj query = query_builder.obj();
        long long count = c->count(helpers::collection_name(Collections::USERS()), query);
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
