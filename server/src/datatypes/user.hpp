/*
 * Created by Natalie Wirth on 05.06.2015.
 * Copyright (c) 2015 Max Planck Institute for Computer Science. All rights reserved.
 */

#ifndef USER_H
#define	USER_H

#include "../extras/utils.hpp"

#include <string>
#include <map>
#include <vector>
#include <mongo/bson/bson.h>

namespace epidb {
  namespace datatypes {

    enum PermissionLevel {
      ADMIN,
      INCLUDE_COLLECTION_TERMS,
      INCLUDE_EXPERIMENTS,
      INCLUDE_ANNOTATIONS,
      GET_DATA,
      LIST_COLLECTIONS,
      NONE,
      NOT_SET
    };

    class User {
    public:
      User();
      User(std::string _id, std::string key, std::string name, std::string email,
           std::string institution);
      User(std::string name, std::string email, std::string institution);
      User(std::vector<mongo::BSONObj> bsonobj);
      User(const User& orig);
      virtual ~User();

      static const std::string PREFIX;
      static const std::string COLLECTION;
      static const std::string FIELD_ID;
      static const std::string FIELD_KEY;
      static const std::string FIELD_NAME;
      static const std::string FIELD_EMAIL;
      static const std::string FIELD_INSTITUTION;
      static const std::string FIELD_PASSWORD;
      static const std::string FIELD_ADMIN;
      static const std::string FIELD_MEMORY_LIMIT;
      static const std::string FIELD_PERMISSION_LEVEL;

      static const size_t KEY_LENGTH;

      void write_to_BSONObjBuilder(mongo::BSONObjBuilder& builder);

      std::string get_id() const;
      std::string get_key() const;
      std::string get_name() const;
      utils::IdName get_id_name() const;
      std::string get_email() const;
      std::string get_institution() const;
      std::string get_password() const;
      long long get_memory_limit() const;
      PermissionLevel get_permission_level() const;
      bool is_admin() const;

      void set_id(std::string id_);
      void set_key(std::string key_);
      void set_name(std::string name_);
      void set_email(std::string email_);
      void set_institution(std::string institution_);
      void set_password(std::string password_);
      void set_memory_limit(long long memory_limit);
      void set_permission_level(PermissionLevel permission_level);

      void generate_key();
      bool generate_id(std::string msg);

      bool has_permission(PermissionLevel permission);

    private:
      void set_permission_level(int permission_level);

      std::string id;
      std::string key;
      std::string name;
      std::string email;
      std::string institution;
      std::string password;
      bool admin = false;
      long long memory_limit = -1;
      PermissionLevel permission_level = NOT_SET;

      static int seed;

    };

  }
}

#endif	/* USER_H */

