//
//  users.cpp
//  epidb
//
//  Created by Felipe Albrecht on 03.11.14.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include <string>

#include <mongo/bson/bson.h>
#include <mongo/client/dbclient.h>

#include "collections.hpp"
#include "config.hpp"
#include "helpers.hpp"

namespace epidb {
  namespace dba {
    namespace users {

      bool is_valid_email(const std::string &email, std::string &msg)
      {
        bool exists = true;
        if (!helpers::check_exist(Collections::USERS(), "email", email, exists, msg)) {
          return false;
        }
        if (exists) {
          std::stringstream ss;
          ss << "Email '" << email << "' is already being used.";
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

        mongo::ScopedDbConnection c(config::get_mongodb_server());
        c->insert(helpers::collection_name(Collections::USERS()), cu);
        if (!c->getLastError().empty()) {
          msg = c->getLastError();
          c.done();
          return false;
        }

        mongo::BSONObjBuilder index_name;
        index_name.append("key", 1);
        c->ensureIndex(helpers::collection_name(Collections::USERS()), index_name.obj());
        if (!c->getLastError().empty()) {
          msg = c->getLastError();
          c.done();
          return false;
        }

        c.done();
        return true;
      }

      bool check_user(const std::string &user_key, bool &r, std::string &msg)
      {
        return helpers::check_exist(Collections::USERS(), "key", user_key, r, msg);
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

      const std::string build_pattern_annotation_name(const std::string &pattern, const std::string &genome, const bool overlap)
      {
        std::string name = "Pattern " + pattern;
        if (overlap) {
          name = name + " (overlap)";
        } else {
          name = name + " (non-overlap)";
        }
        return name + " in the genome " + genome;
      }

      bool set_user_admin(const std::string &user_id, const bool value, std::string &msg)
      {
        mongo::BSONObj o = BSON("findandmodify" << Collections::USERS() <<
                                "query" << BSON("_id" << user_id) <<
                                "update" << BSON("$set" << BSON("admin" << value)));

        mongo::ScopedDbConnection c(config::get_mongodb_server());
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
        mongo::ScopedDbConnection c(config::get_mongodb_server());

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
