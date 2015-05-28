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

      /**
       * Permissions
       */
      bool set_user_admin(const std::string &user_id, const bool value, std::string &msg);

      bool is_admin_key(const std::string &admin_key, bool &ret, std::string &msg);

      /*
      * \brief Cleans cache
      */
      void invalidate_cache();
    }
  }
}

#endif
