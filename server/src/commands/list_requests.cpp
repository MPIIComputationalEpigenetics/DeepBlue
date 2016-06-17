//
//  list_requests.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Natalie Wirth on 28.04.15.
//  Copyright (c) 2016 Max Planck Institute for Informatics. All rights reserved.

//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.

//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.

//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#include <vector>

#include "../engine/commands.hpp"
#include "../engine/engine.hpp"

#include "../dba/collections.hpp"
#include "../datatypes/user.hpp"

#include "../errors.hpp"

#include "../mdbq/common.hpp"
#include "../mdbq/hub.hpp"

namespace epidb {
  namespace command {

    class ListRequestsCommand : public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::REQUESTS, "Lists the Requests made by an user. It is possible to obtain only the requests of a given state.");
      }

      static Parameters parameters_()
      {
        Parameter p[] = {
          Parameter("request_state", serialize::STRING, "Name of the state to get requests for. The valid states are: new, running, done, and failed."),
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
        datatypes::User user;

        if (!check_permissions(user_key, datatypes::GET_DATA, user, msg )) {
          result.add_error(msg);
          return false;
        }

        if (std::find(allowed_status.begin(), allowed_status.end(), status_find) == allowed_status.end()) {
          result.add_error(status_find + " is not an allowed data name. Allowed data names are: " +
                           allowed_status_string);
          return false;
        }

        std::vector<request::Job> jobs;

        if (! Engine::instance().request_jobs(status_find, user_key, jobs, msg)) {
          return false;
        }

        result.set_as_array(true);

        for (const request::Job& job : jobs) {
          if ((job.status.state != "canceled" && job.status.state != "removed") ||
              (status_find == "canceled" || status_find == "removed")) {
            std::vector<serialize::ParameterPtr> list;
            list.push_back(serialize::ParameterPtr(new serialize::SimpleParameter(serialize::STRING, job._id)));
            list.push_back(serialize::ParameterPtr(new serialize::SimpleParameter(serialize::STRING, job.status.state)));
            result.add_list(list);
          }
        }

        return true;
      }
    } listRequestsCommand;
  }
}
