//
//  users.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 03.11.14.
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

#include "../datatypes/user.hpp"

#include "../dba/collections.hpp"

#include "collections.hpp"
#include "exists.hpp"
#include "helpers.hpp"

#include "../errors.hpp"
#include "users.hpp"

namespace epidb {
  namespace dba {
    namespace users {

      class NameCache {
      private:

        std::map<std::string, std::string> id_name;

      public:

        std::string get_user_name(const std::string &id)
        {
          return id_name[id];
        }

        void set_user_name(const std::string &id, const std::string &name)
        {
          id_name[id] = name;
        }

        bool exists_user_id(const std::string &id)
        {
          if (id_name.find(id) != id_name.end()) {
            return true;
          }
          return false;
        }

        void invalidate()
        {
          id_name.clear();
        }
      };

      NameCache name_cache;

      bool add_user(datatypes::User& user, std::string& msg)
      {
        mongo::BSONObjBuilder create_user_builder;
        user.write_to_BSONObjBuilder(create_user_builder);

        int result;
        if (!dba::helpers::get_increment_counter("users", result, msg)) {
          return false;
        }
        user.set_id(datatypes::User::PREFIX + std::to_string(result));
        create_user_builder.append(datatypes::User::FIELD_ID, user.get_id());

        mongo::BSONObj cu = create_user_builder.obj();

        Connection c;
        c->insert(dba::helpers::collection_name(dba::Collections::USERS()), cu);
        if (!c->getLastError().empty()) {
          msg = c->getLastError();
          c.done();
          return false;
        }

        mongo::BSONObjBuilder index_name;
        index_name.append("key", 1);
        c->createIndex(dba::helpers::collection_name(dba::Collections::USERS()), index_name.obj());
        if (!c->getLastError().empty()) {
          msg = c->getLastError();
          c.done();
          return false;
        }

        c.done();
        return true;
      }

      bool modify_user(datatypes::User& user, std::string& msg)
      {
        mongo::BSONObjBuilder create_user_builder;
        user.write_to_BSONObjBuilder(create_user_builder);

        Connection c;
        c->update(dba::helpers::collection_name(dba::Collections::USERS()),
                  BSON("_id" << user.get_id()),
                  BSON("$set" << create_user_builder.obj()));
        if (!c->getLastError().empty()) {
          msg = c->getLastError();
          c.done();
          return false;
        }
        c.done();
        return true;
      }

      bool remove_user(const datatypes::User& user, std::string& msg)
      {
        if (!dba::helpers::remove_one(helpers::collection_name(datatypes::User::COLLECTION), user.get_id(), msg)) {
          return false;
        }
        return true;
      }

      bool get_user_by_key(const std::string& key, datatypes::User& user, std::string& msg)
      {
        mongo::BSONObj result;
        if (!dba::helpers::get_one(datatypes::User::COLLECTION, BSON(datatypes::User::FIELD_KEY << key), result)) {
          msg = Error::m(ERR_INVALID_USER_KEY);
          return false;
        }
        user = datatypes::User(result);
        return true;
      }

      bool get_user_by_email(const std::string& email, const std::string& password, datatypes::User& user, std::string& msg)
      {
        mongo::BSONObj result;
        if (!dba::helpers::get_one(datatypes::User::COLLECTION,
                                   BSON(datatypes::User::FIELD_EMAIL << email << datatypes::User::FIELD_PASSWORD << password),
                                   result)) {
          msg = Error::m(ERR_INVALID_USER_EMAIL_PASSWORD);
          return false;
        }
        user = datatypes::User(result);
        return true;
      }

      bool get_user_by_id(const std::string& id, datatypes::User& user, std::string& msg)
      {
        mongo::BSONObj result;
        if (!dba::helpers::get_one(datatypes::User::COLLECTION, BSON(datatypes::User::FIELD_ID << id), result)) {
          msg = Error::m(ERR_INVALID_USER_ID, id);
          return false;
        }
        user = datatypes::User(result);
        return true;
      }

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

      bool get_id(const std::string &user, std::string& id, std::string &msg)
      {
        if (utils::is_id(user, "u")) {
          id = user;
          return true;
        }

        mongo::BSONObj obj;
        if (!dba::helpers::get_one(dba::Collections::USERS(), BSON("name" << user), obj)) {
          msg = Error::m(ERR_INVALID_USER_NAME, user);
          return false;
        }

        id = obj["_id"].str();
        return true;
      }

      bool get_user(const std::string &user_key, utils::IdName &id_name, std::string &msg)
      {
        return helpers::get_name(Collections::USERS(), user_key, id_name, msg);
      }

      bool get_user_name_by_id(const std::string &user_id, std::string &user_name, std::string &msg)
      {
        if (name_cache.exists_user_id(user_id)) {
          user_name = name_cache.get_user_name(user_id);
          return true;
        } else {
          mongo::BSONObj result;
          if (!helpers::get_one(Collections::USERS(), BSON("_id" <<  user_id), result)) {
            msg = Error::m(ERR_INVALID_USER_ID, user_id);
          }

          user_name = result["name"].str();
          name_cache.set_user_name(user_id, user_name);
        }
        return true;
      }

      void invalidate_cache()
      {
        name_cache.invalidate();
      }

      bool get_owner(const std::string& id, datatypes::User& user, std::string& msg)
      {
        std::string collection;
        if (!dba::Collections::get_collection_for_id(id, collection)) {
          msg = "Datatype of '" + id + "' not accepted for this function";
          return false;
        }
        mongo::BSONObj result;
        if (!helpers::get_one(collection, mongo::Query(BSON("_id" << id)), result)) {
          msg = "Could not get data from database";
          return false;
        }
        std::string user_id;
        if (result.hasField("user")) {
          user_id = result["user"].str();
        } else if (result.hasField("upload_info")) {
          user_id = result["upload_info"]["user"].str();
        } else {
          return false;
        }
        if (!get_user_by_id(user_id, user, msg)) {
          return false;
        }
        return true;
      }
    }
  }
}
