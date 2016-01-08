//
//  users.hpp
//  epidb
//
//  Created by Felipe Albrecht on 03.11.14.
//  Copyright (c) 2016 Max Planck Institute for Informatics. All rights reserved.
//

#ifndef EPIDB_DBA_USERS_HPP
#define EPIDB_DBA_USERS_HPP

#include <string>

#include "../datatypes/user.hpp"
#include "../extras/utils.hpp"

namespace epidb {
  namespace dba {
    namespace users {

      bool add_user(datatypes::User& user, std::string& msg);

      bool modify_user(datatypes::User& user, std::string& msg);

      bool remove_user(const datatypes::User& user, std::string& msg);

      bool get_user_by_key(const std::string& key, datatypes::User& user, std::string& msg);

      bool get_user_by_email(const std::string& email, const std::string& password, datatypes::User& user, std::string& msg);

      bool get_user_by_id(const std::string& id, datatypes::User& user, std::string& msg);

      bool is_valid_email(const std::string &email, std::string &msg);

      /*
       * \brief give the user name or Id and receive the ID. Useful for reading some commands inputs
       */
      bool get_id(const std::string &user, std::string& id, std::string &msg);

      /*
      * \brief  Get IdName for given user-key
      * \param  user_key  The user-key
      *         id_name   Object with the user id and name
      */
      bool get_user(const std::string &user_key, utils::IdName &id_name, std::string &msg);

      /*
      * \brief  Get user-name for given user-ID
      * \param  user_id     The user-ID
      *        user_name   Return: The user-key
      */
      bool get_user_name_by_id(const std::string &user_id, std::string &user_name, std::string &msg);


      bool bind_user(const std::string &email, const std::string &password, const std::string &user_key, const std::string &admin_key, std::string &id, std::string &msg);

      /*
      * \brief Cleans cache
      */
      void invalidate_cache();

      bool get_owner(const std::string& id, datatypes::User& user, std::string& msg);
    }
  }
}

#endif
