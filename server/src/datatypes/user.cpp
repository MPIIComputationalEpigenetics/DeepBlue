/*
 * Created by Natalie Wirth on 05.06.2015.
 * Copyright (c) 2015 Max Planck Institute for Computer Science. All rights reserved.
 */

#include <string>
#include <sstream>
#include <iostream>

#include <boost/random.hpp>

#include "user.hpp"

#include "../dba/config.hpp"
#include "../dba/helpers.hpp"

namespace epidb {
  namespace datatypes {

    const std::string User::PREFIX = "u";
    const std::string User::COLLECTION = "users";
    const std::string User::FIELD_ID = "_id";
    const std::string User::FIELD_KEY = "key";
    const std::string User::FIELD_NAME = "name";
    const std::string User::FIELD_EMAIL = "email";
    const std::string User::FIELD_INSTITUTION = "institution";
    const std::string User::FIELD_PASSWORD = "password";
    const std::string User::FIELD_MEMORY_LIMIT = "memory_limit";
    const std::string User::FIELD_PERMISSION_LEVEL = "permission_level";
    const size_t User::KEY_LENGTH = 16;

    int User::seed = rand();

    User::User()
    {
    }

    User::User(std::string name, std::string email, std::string institution)
    {
      set_name(name);
      set_email(email);
      set_institution(institution);
      generate_key();
    }

    User::User(std::vector<mongo::BSONObj> bsonobj)
    {
      set_name(bsonobj[0][FIELD_NAME].str());
      set_email(bsonobj[0][FIELD_EMAIL].str());
      set_institution(bsonobj[0][FIELD_INSTITUTION].str());
      set_id(bsonobj[0][FIELD_ID].str());
      set_key(bsonobj[0][FIELD_KEY].str());
      set_password(bsonobj[0][FIELD_PASSWORD].str());
      if (bsonobj[0].hasElement(FIELD_PERMISSION_LEVEL)) {
        set_permission_level(bsonobj[0][FIELD_PERMISSION_LEVEL].Int());
      }
      if (bsonobj[0].hasElement(FIELD_MEMORY_LIMIT)) {
        memory_limit = bsonobj[0][FIELD_MEMORY_LIMIT].Long();
      }
    }

    User::User(const User& orig)
    {
    }

    User::~User()
    {
    }

    void User::write_to_BSONObjBuilder(mongo::BSONObjBuilder& builder)
    {
      builder.append(FIELD_KEY, get_key());
      builder.append(FIELD_NAME, get_name());
      builder.append(FIELD_EMAIL, get_email());
      builder.append(FIELD_INSTITUTION, get_institution());
      builder.append(FIELD_PASSWORD, get_password());
      if (memory_limit != -1) {
        builder.append(FIELD_MEMORY_LIMIT, get_memory_limit());
      }
      if (permission_level != NOT_SET) {
        builder.append(FIELD_PERMISSION_LEVEL, get_permission_level());
      }
    }

    void User::generate_key()
    {
      static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

      srand(time(NULL)^seed);
      seed = rand();

      std::stringstream ss;
      for (size_t i = 0; i < KEY_LENGTH; ++i) {
        ss << alphanum[rand() % (sizeof(alphanum) - 1)];
      }

      key = ss.str();
    }

    bool User::has_permission(PermissionLevel permission)
    {
      return static_cast<int>(get_permission_level()) <= static_cast<int>(permission);
    }

    void User::set_id(std::string id)
    {
      this->id = id;
    }
    void User::set_key(std::string key)
    {
      this->key = key;
    }
    void User::set_name(std::string name)
    {
      this->name = name;
    }
    void User::set_email(std::string email)
    {
      this->email = email;
    }
    void User::set_institution(std::string institution)
    {
      this->institution = institution;
    }

    void User::set_password(std::string password)
    {
      this->password = password;
    }

    void User::set_memory_limit(long long memory_limit)
    {
      this->memory_limit = memory_limit;
    }

    void User::set_permission_level(PermissionLevel permission_level)
    {
      this->permission_level = permission_level;
    }

    void User::set_permission_level(int permission_level)
    {
      set_permission_level(static_cast<PermissionLevel>(permission_level));
    }

    std::string User::get_id() const
    {
      return id;
    }

    std::string User::get_key() const
    {
      return key;
    }

    std::string User::get_name() const
    {
      return name;
    }

    utils::IdName User::get_id_name() const
    {
      return utils::IdName(this->get_id(), this->get_name());
    }

    std::string User::get_email() const
    {
      return email;
    }

    std::string User::get_institution() const
    {
      return institution;
    }

    std::string User::get_password() const
    {
      return password;
    }

    long long User::get_memory_limit() const
    {
      if (this->memory_limit == -1) {
        return dba::config::processing_max_memory;
      } else {
        return this->memory_limit;
      }
    }

    PermissionLevel User::get_permission_level() const
    {
      if (permission_level == NOT_SET) {
        return LIST_COLLECTIONS;
      } else {
        return permission_level;
      }
    }

    bool User::is_admin() const
    {
      return get_permission_level() == PermissionLevel::ADMIN;
    }
  }
}
