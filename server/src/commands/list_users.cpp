//
//  list_users.cpp
//  epidb
//
//  Created by Felipe Albrecht on 25.06.13.
//  Copyright (c) 2016 Max Planck Institute for Informatics. All rights reserved.
//

#include "../engine/commands.hpp"

#include "../dba/dba.hpp"
#include "../dba/list.hpp"

#include "../datatypes/user.hpp"

#include "../extras/utils.hpp"
#include "../extras/serialize.hpp"

#include "../errors.hpp"

namespace epidb {
  namespace command {

    class ListUsers: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::ADMINISTRATION, "Lists all existing users.");
      }

      static Parameters parameters_()
      {
        Parameter p[] = {
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 1);
        return params;
      }

      static Parameters results_()
      {
        Parameter p[] = {
          Parameter("users", serialize::LIST, "user names")
        };
        Parameters results(&p[0], &p[0] + 1);
        return results;
      }

    public:
      ListUsers() : Command("list_users", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        const std::string admin_key = parameters[0]->as_string();

        std::string msg;
        datatypes::User user;

        if (!check_permissions(admin_key, datatypes::ADMIN, user, msg )) {
          result.add_error(msg);
          return false;
        }

        std::vector<utils::IdName> names;
        bool ret = dba::list::users(user.get_key(), names, msg);

        if (!ret) {
          result.add_error(msg);
          return false;
        }

        set_id_names_return(names, result);

        return true;
      }
    } listUsers;
  }
}
