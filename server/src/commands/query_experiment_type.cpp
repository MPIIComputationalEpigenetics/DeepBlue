//
//  query_experiment_type.cpp
//  epidb
//
//  Created by Felipe Albrecht on 24.09.2015.
//  Copyright (c) 2015 Max Planck Institute for Computer Science. All rights reserved.
//

#include <map>
#include <sstream>
#include <iostream>

#include "../datatypes/user.hpp"

#include "../dba/dba.hpp"
#include "../dba/queries.hpp"

#include "../engine/commands.hpp"
#include "../engine/engine.hpp"
#include "../engine/request.hpp"

#include "../extras/serialize.hpp"

#include "../errors.hpp"

namespace epidb {
  namespace command {

    class QueryExperimentTypeCommand : public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::OPERATIONS,
                                  "Return information for the given ID (or IDs).");
      }

      static  Parameters parameters_()
      {
        Parameter p[] = {
          Parameter("query_id", serialize::STRING, "query ID"),
          Parameter("type", serialize::STRING, "experiment type (peaks or signal)"),
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 3);
        return params;
      }

      static Parameters results_()
      {
        Parameter p[] = {
          Parameter("information", serialize::STRING, "New query ID.")
        };
        Parameters results(&p[0], &p[0] + 1);
        return results;
      }

    public:
      QueryExperimentTypeCommand() : Command("query_experiment_type", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        const std::string query_id = parameters[0]->as_string();
        const std::string type = parameters[1]->as_string();
        const std::string user_key = parameters[2]->as_string();

        std::string msg;
        datatypes::User user;

        std::string err_msg;
        if (!check_permissions(user_key, datatypes::GET_DATA, user, msg )) {
          result.add_error(msg);
          return false;
        }

        if (type != "signal" && type != "peaks") {
          result.add_error("The type must be 'peaks' or signal'");
          return false;
        }

        std::string new_query_id;
        if (!dba::query::modify_query(query_id, "upload_info.content_format", type, user_key, new_query_id, msg)) {
          result.add_error(msg);
          return false;
        }

        result.add_string(new_query_id);
        return true;
      }
    } queryExperimentTypeCommand;
  }
}
