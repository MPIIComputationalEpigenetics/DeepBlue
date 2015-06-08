/*
 * Created by Natalie Wirth on 05.06.2015.
 * Copyright (c) 2015 Max Planck Institute for Computer Science. All rights reserved.
 */

#include <string>
#include <sstream>
#include <iostream>

#include <boost/random.hpp>

#include "User.hpp"

#include "../dba/helpers.hpp"

namespace epidb {
    
const std::string User::PREFIX = "u";
const std::string User::COLLECTION = "users";
const std::string User::FIELD_ID = "_id";
const std::string User::FIELD_KEY = "key";
const std::string User::FIELD_NAME = "name";
const std::string User::FIELD_EMAIL = "email";
const std::string User::FIELD_INSTITUTION = "institution";
const std::string User::FIELD_ADMIN = "admin";
const size_t User::KEY_LENGTH = 16;

int User::seed = rand();

User::User() {}

User::User(std::string id, std::string key, std::string name, std::string email,
            std::string institution) {
    set_id(id);
    set_key(key);
    User(name, email, institution);
}

User::User(std::string name, std::string email, std::string institution) {
    set_name(name);
    set_email(email);
    set_institution(institution);
    generate_key();
}

User::User(std::vector<mongo::BSONObj> bsonobj) {
    User(bsonobj[0][FIELD_NAME].str(),
            bsonobj[0][FIELD_EMAIL].str(), 
            bsonobj[0][FIELD_INSTITUTION].str());
    set_id(bsonobj[0][FIELD_ID].str());
    set_key(bsonobj[0][FIELD_KEY].str());
    admin = bsonobj[0][FIELD_ADMIN].Bool();
}

User::User(const User& orig) {
}

User::~User() {
}

std::map<std::string, std::string> User::get_fields() {
    std::map<std::string, std::string> fields;
    fields[FIELD_KEY] = key;
    fields[FIELD_NAME] = name;
    fields[FIELD_EMAIL] = email;
    fields[FIELD_INSTITUTION] = institution;
    return fields;
}

void User::generate_key() {
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

bool User::has_permission(Permission permission) {
    switch(permission) {
        case ADMIN : return admin == true; break;
        default: return false;
    }
}

void User::set_id(std::string id) {this->id = id;}
void User::set_key(std::string key) {this->key = key;}
void User::set_name(std::string name) {this->name = name;}
void User::set_email(std::string email) {this->email = email;}
void User::set_institution(std::string institution) {this->institution = institution;}

std::string User::get_id() {return id;}
std::string User::get_key() {return key;}
std::string User::get_name() {return name;}
std::string User::get_email() {return email;}
std::string User::get_institution() {return institution;}

}