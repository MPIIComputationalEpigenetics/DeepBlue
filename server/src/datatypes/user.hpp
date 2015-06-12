/*
 * Created by Natalie Wirth on 05.06.2015.
 * Copyright (c) 2015 Max Planck Institute for Computer Science. All rights reserved.
 */

#ifndef USER_H
#define	USER_H

#include <string>
#include <map>
#include <vector>
#include <mongo/bson/bson.h>

namespace epidb {
  namespace datatypes {

    enum PermissionLevel {
      ADMIN,
      INCLUDE_CV_TERMS,
      INCLUDE_EXPERIMENTS,
      INCLUDE_ANNOTATIONS,
      GET_DATA,
      LIST_COLLECTIONS,
      NONE
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
      static const std::string FIELD_ADMIN;
      static const size_t KEY_LENGTH;

      std::map<std::string, std::string> get_fields();

      std::string get_id();
      std::string get_key();
      std::string get_name();
      std::string get_email();
      std::string get_institution();

      void set_id(std::string id_);
      void set_key(std::string key_);
      void set_name(std::string name_);
      void set_email(std::string email_);
      void set_institution(std::string institution_);

      void generate_key();
      bool generate_id(std::string msg);

      bool has_permission(PermissionLevel permission);

    private:
      std::string id;
      std::string key;
      std::string name;
      std::string email;
      std::string institution;
      bool admin;

      static int seed;

    };

  }
}

#endif	/* USER_H */

