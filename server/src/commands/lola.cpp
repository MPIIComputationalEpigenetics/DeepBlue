//
//  score_matrix.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 08.11.14.
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
#include "../extras/utils.hpp"

#include "../errors.hpp"
#include "../log.hpp"


namespace epidb {
  namespace command {

    class LolaCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::OPERATIONS, "Build a matrix containing the aggregation result of the the experiments data by the aggregation boundaries.");
      }

      static Parameters parameters_()
      {
        Parameter p[] = {
          parameters::QueryId,
          Parameter("universe_query_id", serialize::STRING, "map with experiments names and columns to be processed. Example : {'wgEncodeBroadHistoneDnd41H3k27acSig.wig':'VALUE', 'wgEncodeBroadHistoneCd20ro01794H3k27acSig.wig':'VALUE'}"),
          Parameter("databases", serialize::MAP, "aggregation function name: min, max, sum, mean, var, sd, median, count, boolean"),
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 4);
        return params;
      }

      static Parameters results_()
      {
        Parameter p[] = {
          Parameter("lola", serialize::STRING, "the score matrix containing the summarized data")
        };
        Parameters results(&p[0], &p[0] + 1);
        return results;
      }

    public:
      LolaCommand() : Command("lola", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        const std::string query_id = parameters[0]->as_string();
        const std::string universe_query_id = parameters[1]->as_string();
        const std::string user_key = parameters[3]->as_string();

        std::unordered_map<std::string, std::vector<std::string>> database_experiments;

        std::map<std::string, serialize::ParameterPtr> databases_;
        if (!parameters[2]->children(databases_)) {
          result.add_error("unable to read metadata");
          return false;
        }

        for (const auto& database: databases_ ) {
          const std::string& name = database.first;
          serialize::ParameterPtr experiments_ptr = database.second;

          std::vector<serialize::ParameterPtr> experiments_list;
          experiments_ptr->children(experiments_list);
          std::vector<std::string> experiments;
          for (const auto& exp_: experiments_list) {
            const std::string& experiment_name = exp_->as_string();
            if (!dba::exists::experiment(utils::normalize_name(experiment_name))) {
              result.add_error(Error::m(ERR_INVALID_EXPERIMENT, experiment_name));
              return false;
            }
            experiments.push_back(experiment_name);
          }
          database_experiments[name] = experiments;
        }

        std::string msg;
        std::string request_id;
        if (!epidb::Engine::instance().queue_lola(query_id, universe_query_id, database_experiments, user_key, request_id, msg)) {
          result.add_error(msg);
          return false;
        }

        result.add_string(request_id);
        return true;
      }

    } lolaCommand;
  }
}
