/*
 * Created by Natalie Wirth on 05.06.2015.
 * Copyright (c) 2015 Max Planck Institute for Computer Science. All rights reserved.
 */

#ifndef USER_H
#define USER_H

#include "../extras/utils.hpp"

#include <string>
#include <map>
#include <vector>
#include <mongo/bson/bson.h>

namespace epidb {
  namespace datatypes {

    enum PermissionLevel {
      ADMIN = 0,
      INCLUDE_COLLECTION_TERMS = 10,
      INCLUDE_EXPERIMENTS = 20,
      INCLUDE_ANNOTATIONS = 30,
      GET_DATA = 40,
      LIST_COLLECTIONS = 50,
      NONE = 1000,
      NOT_SET = 10000
    };


    std::string permission_level_to_string(PermissionLevel pl);


    class User {
    public:
      User();
      User(std::string _id, std::string key, std::string name, std::string email,
           std::string institution);
      User(std::string name, std::string email, std::string institution);
      User(mongo::BSONObj bsonobj, std::vector<std::string>& public_projects);

      static const std::string PREFIX;
      static const std::string FIELD_ID;
      static const std::string FIELD_KEY;
      static const std::string FIELD_NAME;
      static const std::string FIELD_EMAIL;
      static const std::string FIELD_INSTITUTION;
      static const std::string FIELD_PASSWORD;
      static const std::string FIELD_ADMIN;
      static const std::string FIELD_MEMORY_LIMIT;
      static const std::string FIELD_PERMISSION_LEVEL;
      static const std::string FIELD_PROJECTS;

      static const size_t KEY_LENGTH;

      void write_to_BSONObjBuilder(mongo::BSONObjBuilder& builder);

      std::string id() const;
      std::string key() const;
      std::string name() const;
      utils::IdName id_name() const;
      std::string email() const;
      std::string institution() const;
      std::string password() const;
      long long memory_limit() const;
      PermissionLevel permission_level() const;
      std::vector<std::string> projects() const;
      bool is_admin() const;

      void id(std::string id_);
      void key(std::string key_);
      void name(std::string name_);
      void email(std::string email_);
      void institution(std::string institution_);
      void password(std::string password_);
      void memory_limit(long long memory_limit);
      void permission_level(PermissionLevel permission_level);

      void generate_key();
      bool generate_id(std::string msg);

      bool has_permission(PermissionLevel permission);

      static const std::string& ANONYMOUS_USER();
    private:
      void permission_level(int permission_level);

      std::string _id;
      std::string _key;
      std::string _name;
      std::string _email;
      std::string _institution;
      std::string _password;
      long long _memory_limit = -1;
      PermissionLevel _permission_level = NOT_SET;
      std::vector<std::string> _projects_member;
      std::vector<std::string> _all_projects;

      static int _seed;

    };

  }
}

#endif  /* USER_H */

