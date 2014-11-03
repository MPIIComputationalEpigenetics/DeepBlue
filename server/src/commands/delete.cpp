//
//  delete.cpp
//  epidb
//
//  Created by Felipe Albrecht on 19.07.13.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include "../dba/dba.hpp"
#include "../dba/users.hpp"

#include "../extras/serialize.hpp"

#include "../engine/commands.hpp"

namespace epidb {
  namespace command {

    class DeleteCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::ADMINISTRATION, "Remove data from DeepBlue.");
      }

      static  Parameters parameters_()
      {
        Parameter p[] = {
          Parameter("id", serialize::STRING, "Data id to be deleted."),
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 2);
        return params;
      }

      static Parameters results_()
      {
        Parameter p[] = {
          Parameter("id", serialize::STRING, "id of the deleted data")
        };
        Parameters results(&p[0], &p[0] + 1);
        return results;
      }

    public:
      DeleteCommand() : Command("delete", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        const std::string id = parameters[0]->as_string();
        const std::string user_key = parameters[1]->as_string();

        std::string msg;
        if (!Command::checks(user_key, msg)) {
          result.add_error(msg);
          return false;
        }

        bool is_admin_key;
        if (!dba::users::is_admin_key(user_key, is_admin_key, msg)) {
          result.add_error(msg);
          return false;
        }

        result.add_error("command not implemented :(");

        // data::delete_by_id(id, user_key, )

        return false;
      }

    } deleteCommand;
  }
}
