/*
 * Created by Natalie Wirth on 13.06.2015.
 * Copyright (c) 2015 Max Planck Institute for Computer Science. All rights reserved.
 */

#ifndef USER_HPP
#define	USER_HPP

#include "../datatypes/user.hpp"

namespace epidb {
  namespace dba {
    bool add_user(datatypes::User& user, std::string& msg);
    bool modify_user(datatypes::User& user, std::string& msg);
    bool remove_user(const datatypes::User& user, std::string& msg);
    bool get_user_by_key(const std::string& key, datatypes::User& user, std::string& msg);
    bool get_user_by_email(const std::string& email, const std::string& password, datatypes::User& user, std::string& msg);
    bool get_user_by_id(const std::string& id, datatypes::User& user, std::string& msg);
  }
}
#endif	/* USER_HPP */

