//
//  get_experiments_by_query.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 12.06.13.
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

    class GetExperimentsByQueryCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::OPERATIONS, "Return a list of experiments and annotations that have at least one region in the data set represented by the query.");
      }

      static Parameters parameters_()
      {
        Parameter p[] = {
          Parameter("query_id", serialize::STRING, "id of the query"),
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 2);
        return params;
      }

      static Parameters results_()
      {
        Parameter p[] = {
          Parameter("experiments", serialize::LIST, "List containing experiments names and ids")
        };
        Parameters results(&p[0], &p[0] + 1);
        return results;
      }

    public:
      GetExperimentsByQueryCommand() : Command("get_experiments_by_query", parameters_(), results_(), desc_()) {}

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
          if (msg.empty()) {
            result.add_error("Invalid query ID: " + query_id);
          } else {
            result.add_error(msg);
          }
          return false;
        }

        std::string request_id;
        if (!epidb::Engine::instance().queue_get_experiments_by_query(query_id, user_key, request_id, msg)) {
          result.add_error(msg);
          return false;
        }

        result.add_string(request_id);
        return true;

      }
    } getExperimentsByQueryCommand;
  }
}
