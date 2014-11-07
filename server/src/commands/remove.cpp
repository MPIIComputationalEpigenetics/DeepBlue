//
//  remove.cpp
//  epidb
//
//  Created by Felipe Albrecht on 19.07.13.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include <string>

#include "../dba/users.hpp"
#include "../dba/remove.hpp"

#include "../extras/serialize.hpp"

#include "../engine/commands.hpp"

namespace epidb {
  namespace command {

    class RemoveCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::ADMINISTRATION, "Remove data from DeepBlue.");
      }

      static  Parameters parameters_()
      {
        Parameter p[] = {
          Parameter("id", serialize::STRING, "Data id to be removed."),
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 2);
        return params;
      }

      static Parameters results_()
      {
        Parameter p[] = {
          Parameter("id", serialize::STRING, "id of the removed data")
        };
        Parameters results(&p[0], &p[0] + 1);
        return results;
      }

    public:
      RemoveCommand() : Command("remove", parameters_(), results_(), desc_()) {}

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

        bool ok = false;
        if (id.compare(0, 1, "a") == 0) {
          ok = dba::remove::annotation(id, user_key, msg);
        } else if (id.compare(0, 1, "g") == 0) {
          ok = dba::remove::genome(id, user_key, msg);
        } else if (id.compare(0, 1, "p") == 0) {
          ok = dba::remove::project(id, user_key, msg);
        } else if (id.compare(0, 2, "bs") == 0) {
          ok = dba::remove::biosource(id, user_key, msg);
        } else if (id.compare(0, 1, "s") == 0) {
          ok = dba::remove::sample(id, user_key, msg);
        } else if (id.compare(0, 2, "em") == 0) {
          ok = dba::remove::epigenetic_mark(id, user_key, msg);
        } else if (id.compare(0, 1, "e") == 0) {
          ok = dba::remove::experiment(id, user_key, msg);
        } else if (id.compare(0, 1, "q") == 0) {
          ok = dba::remove::query(id, user_key, msg);
        } else if (id.compare(0, 2, "tr") == 0) {
          ok = dba::remove::tiling_region(id, user_key, msg);
        } else if (id.compare(0, 1, "t") == 0) {
          ok = dba::remove::technique(id, user_key, msg);
        } else if (id.compare(0, 1, "f") == 0) {
          ok = dba::remove::sample_field(id, user_key, msg);
        } else if (id.compare(0, 2, "ct") == 0) {
          ok = dba::remove::column_type(id, user_key, msg);
        } else {
          result.add_error("Invalid identifier: " + id);
          return false;
        }
        if (!ok) {
          result.add_error(msg);
          return false;
        }

        result.add_string(id);
        return true;
      }

    } removeCommand;
  }
}
