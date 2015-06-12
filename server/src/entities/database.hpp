/*
 * Created by Natalie Wirth on 05.06.2015.
 * Copyright (c) 2015 Max Planck Institute for Computer Science. All rights reserved.
 */

#ifndef DATABASE_HPP
#define	DATABASE_HPP

#include <iostream>

#include "../datatypes/user.hpp"

#include "../connection/connection.hpp"
#include "../dba/helpers.hpp"
#include "../dba/collections.hpp"

namespace epidb {
namespace dba {

bool add_user(datatypes::User& user, std::string& msg) {
    mongo::BSONObjBuilder create_user_builder;
    std::map<std::string, std::string> fields = user.get_fields();
    for (auto it = fields.begin(); it != fields.end(); ++it) {
        create_user_builder.append(it->first, it->second);
    }
    
    int result;
    if (!dba::helpers::get_increment_counter("users", result, msg)) {
        return false;
    }
    user.set_id(datatypes::User::PREFIX + std::to_string(result));
    create_user_builder.append("_id", user.get_id());
    
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

bool get_user(const std::string& key, datatypes::User& user, std::string& msg){
    std::vector<mongo::BSONObj> result;
    dba::helpers::get(datatypes::User::COLLECTION, datatypes::User::FIELD_KEY, key, result, msg);
    if (result.size() == 0) {
          msg = "Unable to retrieve user with key %s" + key;
          return false;
    }
    user = datatypes::User(result);
    return true;
}
}
}
#endif	/* DATABASE_HPP */

