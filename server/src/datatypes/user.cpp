/*
 * Created by Natalie Wirth on 05.06.2015.
 * Copyright (c) 2015 Max Planck Institute for Computer Science. All rights reserved.
 */

#include <string>
#include <sstream>
#include <iostream>
#include <vector>

#include <boost/random.hpp>

#include "user.hpp"

#include "../config/config.hpp"
#include "../dba/helpers.hpp"

namespace epidb {
  namespace datatypes {

    const std::string User::PREFIX = "u";
    const std::string User::FIELD_ID = "_id";
    const std::string User::FIELD_KEY = "key";
    const std::string User::FIELD_NAME = "name";
    const std::string User::FIELD_EMAIL = "email";
    const std::string User::FIELD_INSTITUTION = "institution";
    const std::string User::FIELD_PASSWORD = "password";
    const std::string User::FIELD_MEMORY_LIMIT = "memory_limit";
    const std::string User::FIELD_PERMISSION_LEVEL = "permission_level";
    const std::string User::FIELD_PROJECTS = "projects";
    const size_t User::KEY_LENGTH = 16;

    std::string permission_level_to_string(PermissionLevel pl)
    {
      switch (pl) {
      case datatypes::ADMIN:
        return "ADMIN";
      case datatypes::INCLUDE_COLLECTION_TERMS:
        return "INCLUDE_COLLECTION_TERMS";
      case datatypes::INCLUDE_EXPERIMENTS:
        return "INCLUDE_EXPERIMENTS";
      case datatypes::INCLUDE_ANNOTATIONS:
        return "INCLUDE_ANNOTATIONS";
      case datatypes::GET_DATA:
        return "GET_DATA";
      case datatypes::LIST_COLLECTIONS:
        return "LIST_COLLECTIONS";
      case datatypes::NONE:
        return "NONE";
      default:
        return "Unknown permission level: " + utils::integer_to_string(pl);
      }
    }

    int User::_seed = rand();

    User::User()
    {
    }

    User::User(std::string n, std::string e, std::string i):
      _name(n),
      _email(e),
      _institution(i)
    {
    }


    User::User(mongo::BSONObj bsonobj, std::vector<std::string>& public_projects, std::vector<std::string>& private_projects)
    {
      _name = bsonobj[FIELD_NAME].str();
      _email = bsonobj[FIELD_EMAIL].str();
      _institution = bsonobj[FIELD_INSTITUTION].str();
      _id = bsonobj[FIELD_ID].str();
      _key = bsonobj[FIELD_KEY].str();
      _password= bsonobj[FIELD_PASSWORD].str();
      if (bsonobj.hasElement(FIELD_PERMISSION_LEVEL)) {
        _permission_level = static_cast<PermissionLevel>(bsonobj[FIELD_PERMISSION_LEVEL].safeNumberLong());
      }
      if (bsonobj.hasElement(FIELD_MEMORY_LIMIT)) {
        _memory_limit = bsonobj[FIELD_MEMORY_LIMIT].safeNumberLong();
      }

      _projects_member = private_projects;

      std::set<std::string> all;
      all.insert(public_projects.begin(), public_projects.end());
      all.insert(private_projects.begin(), private_projects.end());

      _all_projects = std::vector<std::string>(all.begin(), all.end());
    }

    void User::write_to_BSONObjBuilder(mongo::BSONObjBuilder& builder)
    {
      builder.append(FIELD_KEY, key());
      builder.append(FIELD_NAME, name());
      builder.append(FIELD_EMAIL, email());
      builder.append(FIELD_INSTITUTION, institution());
      builder.append(FIELD_PASSWORD, password());
      builder.append(FIELD_PERMISSION_LEVEL, permission_level());

      if (_memory_limit != -1) {
        builder.append(FIELD_MEMORY_LIMIT, memory_limit());
      }

      if (!projects().empty()) {
        builder.append(FIELD_PROJECTS, utils::build_array(projects()));
      }
    }

    void User::generate_key()
    {
      static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

      srand(time(NULL)^_seed);
      _seed = rand();

      std::stringstream ss;
      for (size_t i = 0; i < KEY_LENGTH; ++i) {
        ss << alphanum[rand() % (sizeof(alphanum) - 1)];
      }

      _key = ss.str();
    }

    bool User::has_permission(PermissionLevel permission)
    {
      return static_cast<int>(permission_level()) <= static_cast<int>(permission);
    }

    void User::id(std::string id)
    {
      this->_id = id;
    }
    void User::key(std::string key)
    {
      this->_key = key;
    }
    void User::name(std::string name)
    {
      this->_name = name;
    }
    void User::email(std::string email)
    {
      this->_email = email;
    }
    void User::institution(std::string institution)
    {
      this->_institution = institution;
    }

    void User::password(std::string password)
    {
      this->_password = password;
    }

    void User::memory_limit(long long memory_limit)
    {
      this->_memory_limit = memory_limit;
    }

    void User::permission_level(PermissionLevel permission_level)
    {
      this->_permission_level = permission_level;
    }

    void User::permission_level(int permission_level)
    {
      _permission_level = static_cast<PermissionLevel>(permission_level);
    }

    std::string User::id() const
    {
      return _id;
    }

    std::string User::key() const
    {
      return _key;
    }

    std::string User::name() const
    {
      return _name;
    }

    utils::IdName User::id_name() const
    {
      return utils::IdName(this->id(), this->name());
    }

    std::string User::email() const
    {
      return _email;
    }

    std::string User::institution() const
    {
      return _institution;
    }

    std::string User::password() const
    {
      return _password;
    }

    long long User::memory_limit() const
    {
      if (this->_memory_limit == -1) {
        return config::processing_max_memory;
      }
      return this->_memory_limit;
    }

    PermissionLevel User::permission_level() const
    {
      if (_permission_level == NOT_SET) {
        return LIST_COLLECTIONS;
      }
      return _permission_level;
    }

    std::vector<std::string> User::projects() const
    {
      return _all_projects;
    }

    std::vector<std::string> User::projects_member() const
    {
      return _projects_member;
    }

    bool User::is_admin() const
    {
      return permission_level() == PermissionLevel::ADMIN;
    }

    const std::string& User::ANONYMOUS_USER()
    {
      static std::string anonymous("anonymous");
      return anonymous;
    }
  }
}
