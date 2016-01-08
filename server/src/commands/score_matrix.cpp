//
//  score_matrix.cpp
//  epidb
//
//  Created by Felipe Albrecht on 08.11.14.
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

    class ScoreMatrixCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::OPERATIONS, "Gets the regions for the given query in the requested BED format.");
      }

      static Parameters parameters_()
      {
        Parameter p[] = {
          Parameter("experiments_format", serialize::MAP, "map with experiments names and columns to be processed. Example (python): {'wgEncodeBroadHistoneDnd41H3k27acSig.wig':'VALUE', 'wgEncodeBroadHistoneCd20ro01794H3k27acSig.wig':'VALUE'}"),
          Parameter("aggregation_function", serialize::STRING, "aggregation function name: min, max, mean, var, sd, median, count, boolean"),
          Parameter("query_id", serialize::STRING, "regions query id"),
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 4);
        return params;
      }

      static Parameters results_()
      {
        Parameter p[] = {
          Parameter("regions", serialize::STRING, "BED formated regions")
        };
        Parameters results(&p[0], &p[0] + 1);
        return results;
      }

    public:
      ScoreMatrixCommand() : Command("score_matrix", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        const std::string aggregation_function = parameters[1]->as_string();
        const std::string regions_query_id = parameters[2]->as_string();
        const std::string user_key = parameters[3]->as_string();

        std::string msg;
        datatypes::User user;

        if (!check_permissions(user_key, datatypes::GET_DATA, user, msg )) {
          result.add_error(msg);
          return false;
        }

        if (!dba::exists::query(regions_query_id, user_key, msg)) {
          if (msg.empty()) {
            result.add_error(Error::m(ERR_INVALID_QUERY_ID, regions_query_id));
          } else {
            result.add_error(msg);
          }
          return false;
        }

        // TODO Check same experiments biosource
        std::map<std::string, serialize::ParameterPtr> map_;
        if (!parameters[0]->children(map_)) {
          result.add_error("unable to read metadata");
          return false;
        }
        std::vector<std::pair<std::string, std::string>> experiments_formats;
        std::map<std::string, serialize::ParameterPtr>::iterator mit;
        for (mit = map_.begin(); mit != map_.end(); ++mit) {
          std::pair<std::string, std::string> p(mit->first, mit->second->as_string());
          experiments_formats.push_back(p);
        }

        std::string request_id;
        if (!epidb::Engine::instance().queue_score_matrix(experiments_formats, aggregation_function, regions_query_id, user_key, request_id, msg)) {
          result.add_error(msg);
          return false;
        }

        result.add_string(request_id);
        return true;
      }

    } scoreMatrixCommand;
  }
}
