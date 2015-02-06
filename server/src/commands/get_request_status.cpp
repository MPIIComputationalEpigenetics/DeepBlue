//
//  get_request_status.cpp
//  epidb
//
//  Created by Felipe Albrecht on 27.01.15.
//  Copyright (c) 2013,2014,2015 Max Planck Institute for Computer Science. All rights reserved.
//

#include <sstream>
#include <map>

#include "../dba/dba.hpp"
#include "../dba/queries.hpp"

#include "../extras/serialize.hpp"

#include "../engine/commands.hpp"
#include "../engine/engine.hpp"
#include "../engine/request.hpp"

#include "../errors.hpp"

namespace epidb {
  namespace command {

    class GetRequestStatusCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::REQUESTS, "Get the status of the given request.");
      }

      static  Parameters parameters_()
      {
        Parameter p[] = {
          Parameter("request_id", serialize::STRING, "ID of the request"),
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 2);
        return params;
      }

      static Parameters results_()
      {
        Parameter p[] = {
          Parameter("status", serialize::STRING, "request_status: waiting,running,done,error"),
          Parameter("msg", serialize::STRING, "message: can be empty, an error message or actual status.")
        };
        Parameters results(&p[0], &p[0] + 2);
        return results;
      }

    public:
      GetRequestStatusCommand() : Command("get_request_status", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        const std::string query_id = parameters[0]->as_string();
        const std::string user_key = parameters[1]->as_string();

        std::string msg;
        if (!Command::checks(user_key, msg)) {
          result.add_error(msg);
          return false;
        }

        request::Status request_status;
        if (!epidb::Engine::instance().request_status(query_id, user_key, request_status, msg)) {
          result.add_error(msg);
          return false;
        }

        result.add_string(request_status.state);
        result.add_string(request_status.message);

        return true;
      }
    } getRequestStatusCommand;
  }
}
