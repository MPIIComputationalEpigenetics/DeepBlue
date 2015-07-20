//
//  count_regions.cpp
//  epidb
//
//  Created by Felipe Albrecht on 29.08.13.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include "../dba/dba.hpp"
#include "../dba/exists.hpp"
#include "../datatypes/user.hpp"
#include "../engine/commands.hpp"
#include "../engine/engine.hpp"

#include "../extras/serialize.hpp"

#include "../errors.hpp"
#include "../log.hpp"

namespace epidb {
  namespace command {

    class CountRegionsCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::OPERATIONS, "Send a request to counts the number of regions in the result of the given query.");
      }

      static  Parameters parameters_()
      {
        Parameter p[] = {
          Parameter("query_id", serialize::STRING, "Query ID"),
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 2);
        return params;
      }

      static Parameters results_()
      {
        Parameter p[] = {
          Parameter("request_id", serialize::STRING, "Request ID - Use it to retrieve the result with get_request_status and get_request_data")
        };
        Parameters results(&p[0], &p[0] + 1);
        return results;
      }

    public:
      CountRegionsCommand() : Command("count_regions", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        const std::string query_id = parameters[0]->as_string();
        const std::string user_key = parameters[1]->as_string();

        std::string msg;
        datatypes::User user;

        if (!check_permissions(user_key, datatypes::GET_DATA, user, msg )) {
          result.add_error(msg);
          return false;
        }

        if (!dba::exists::query(query_id, user_key, msg)) {
          result.add_error("Invalid query id: '" + query_id + "'" + msg);
          return false;
        }

        std::string request_id;
        if (!epidb::Engine::instance().queue_count_regions(query_id, user_key, request_id, msg)) {
          result.add_error(msg);
          return false;
        }

        result.add_string(request_id);
        return true;
      }
    } countRegionsCommand;
  }
}
