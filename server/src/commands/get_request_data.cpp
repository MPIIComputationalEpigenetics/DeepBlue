//
//  get_request_data.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 27.01.15.
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

#include <iostream>
#include <sstream>
#include <map>

#include "../dba/dba.hpp"
#include "../dba/queries.hpp"
#include "../dba/users.hpp"

#include "../datatypes/user.hpp"

#include "../engine/commands.hpp"
#include "../engine/engine.hpp"

#include "../extras/serialize.hpp"
#include "../extras/utils.hpp"

#include "../errors.hpp"

namespace epidb {
  namespace command {

    class GetRequestDataCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::REQUESTS, "Download the requested data. The output can be (i) a string (get_regions, score_matrix, and count_regions), or (ii) a list of ID and names (get_experiments_by_query), or (iii) a struct (coverage).");
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
          Parameter("data", serialize::STRING, "the request data", true)
        };
        Parameters results(&p[0], &p[0] + 1);
        return results;
      }

    public:
      GetRequestDataCommand() : Command("get_request_data", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        const std::string request_id = parameters[0]->as_string();
        const std::string user_key = parameters[1]->as_string();

        std::string msg;
        datatypes::User user;

        if (!check_permissions(user_key, datatypes::GET_DATA, user, msg )) {
          result.add_error(msg);
          return false;
        }

        std::string file_content;
        if (!epidb::Engine::instance().user_owns_request(request_id, user.id())) {
          result.add_error("Request ID " + request_id + " not found.");
          return false;
        }

        return epidb::Engine::instance().request_data(user, request_id, result);
      }
    } getRequestDataCommand;
  }
}
