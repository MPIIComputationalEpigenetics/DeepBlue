//
//  list_requests.cpp
//  epidb
//
//  Created by Natalie Wirth on 28.04.15.
//  Copyright (c) 2015 Max Planck Institute for Computer Science. All rights reserved.
//

#include <vector>

#include "../engine/commands.hpp"
#include "../engine/engine.hpp"
#include "../engine/request.hpp"

#include "../dba/collections.hpp"

#include "../mdbq/common.hpp"
#include "../mdbq/hub.hpp"

namespace epidb {
  namespace command {

    class ListRequestsCommand : public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::GENERAL_INFORMATION, "Lists all requests in given state.");
      }

      static Parameters parameters_()
      {
        Parameter p[] = {
          Parameter("request_state", serialize::STRING, "Name of the state to get requests for"),
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 2);
        return params;
      }

      static Parameters results_()
      {
        Parameter p[] = {
          Parameter("data_state", serialize::LIST, "Request-IDs and their state")
        };
        Parameters results(&p[0], &p[0] + 1);
        return results;
      }

      std::vector<std::string> allowed_status;
      std::string allowed_status_string;

    public:
      ListRequestsCommand() : Command("list_requests", parameters_(), results_(), desc_())
      {
        for (int i = mdbq::_TS_FIRST, j = 0; i < mdbq::_TS_END; i++, j++) {
          allowed_status.push_back(mdbq::Hub::state_name(i));
          allowed_status_string += allowed_status[j] + ", ";
        }
        allowed_status.push_back("");
        allowed_status_string += "\"\"";
      }

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        const std::string status_find = parameters[0]->as_string();
        const std::string user_key = parameters[1]->as_string();

        std::string msg;
        if (!Command::checks(user_key, msg)) {
          result.add_error(msg);
          return false;
        }

        if(std::find(allowed_status.begin(), allowed_status.end(), status_find) == allowed_status.end()) {
          result.add_error(status_find + " is not an allowed data name. Allowed data names are: " +
                           allowed_status_string);
          return false;
        }

        std::vector<request::Job> jobs;

        if (! Engine::instance().request_jobs(status_find, user_key, jobs, msg)) {
          return false;
        }

        for (const request::Job& job : jobs) {
          std::vector<serialize::ParameterPtr> list;
          list.push_back(serialize::ParameterPtr(new serialize::SimpleParameter(serialize::STRING, job._id)));
          list.push_back(serialize::ParameterPtr(new serialize::SimpleParameter(serialize::STRING, job.status.state)));
          result.add_list(list);
        }

        return true;
      }
    } listRequestsCommand;
  }
}