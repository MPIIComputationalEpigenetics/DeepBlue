//
//  get_regions.cpp
//  epidb
//
//  Created by Felipe Albrecht on 12.06.13.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include "../dba/dba.hpp"
#include "../dba/exists.hpp"

#include "../engine/commands.hpp"
#include "../engine/engine.hpp"

#include "../extras/serialize.hpp"

#include "../errors.hpp"
#include "../log.hpp"

namespace epidb {
  namespace command {

    class GetRegionsCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::OPERATIONS, "Send a request  to retrieve the regions for the given query in the requested BED format.");
      }

      static Parameters parameters_()
      {
        Parameter p[] = {
          Parameter("query_id", serialize::STRING, "Query ID"),
          Parameter("output_format", serialize::STRING, "Output format"),
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 3);
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
      GetRegionsCommand() : Command("get_regions", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        const std::string query_id = parameters[0]->as_string();
        const std::string output_format = parameters[1]->as_string();
        const std::string user_key = parameters[2]->as_string();

        std::string msg;
        if (!Command::checks(user_key, msg)) {
          result.add_error(msg);
          return false;
        }

        if (!dba::exists::query(query_id, user_key, msg)) {
          if (msg.empty()) {
            result.add_error("Invalid query ID: " + query_id);
          } else {
            result.add_error(msg);
          }
          return false;
        }

        std::string request_id;
        if (!epidb::Engine::instance().queue_get_regions(query_id, output_format, user_key, request_id, msg)) {
          result.add_error(msg);
          return false;
        }

        result.add_string(request_id);
        return true;
      }

    } getRegionsCommand;
  }
}
