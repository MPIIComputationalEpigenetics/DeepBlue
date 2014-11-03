//
//  users.hpp
//  epidb
//
//  Created by Felipe Albrecht on 03.11.14.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#ifndef EPIDB_DBA_USERS_HPP
#define EPIDB_DBA_USERS_HPP

#include <string>

#include "../extras/utils.hpp"

namespace epidb {
  namespace dba {
    namespace users {

      bool is_valid_email(const std::string &email, std::string &msg);

      bool add_user(const std::string &name, const std::string &email, const std::string &institution,
                    const std::string &key, std::string &user_id, std::string &msg);

      bool check_user(const std::string &user_key, bool &r, std::string &msg);

      bool get_user_name(const std::string &user_key, std::string &name, std::string &msg);

      bool get_user_name(const std::string &user_key, utils::IdName &id_name, std::string &msg);

      /**
       * Permissions
       */

      bool set_user_admin(const std::string &user_id, const bool value, std::string &msg);

      bool is_admin_key(const std::string &admin_key, bool &ret, std::string &msg);
    }
  }
}

#endif
