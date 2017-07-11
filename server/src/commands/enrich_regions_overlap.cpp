//
//  lola.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 22.06.17.
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

    class EnrichRegionsGoTermsCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::ENRICHMENT, "Enrich the regions based on regions overlap analysis.");
      }

      static Parameters parameters_()
      {
        return {
          parameters::QueryId,
          Parameter("background_query_id", serialize::STRING, "query_id containing the regions that will be used as the background regions"),
          Parameter("databases", serialize::MAP, "map of datasets, where map item is a string with the dataset name and a list with experiment names or query_ids."),
          parameters::Genome,
          parameters::UserKey
        };
      }

      static Parameters results_()
      {
        return {
          Parameter("lola", serialize::STRING, "Request ID - Use it to retrieve the result with info() and get_request_data(). The result is a list containing the datasets that overlap with the query_id regions.")
        };
      }

    public:
      EnrichRegionsGoTermsCommand() : Command("enrich_region_overlap", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        const std::string query_id = parameters[0]->as_string();
        const std::string universe_query_id = parameters[1]->as_string();
        const std::string genome = parameters[3]->as_string();
        const std::string user_key = parameters[4]->as_string();

        std::unordered_map<std::string, std::vector<std::string>> database_experiments;

        std::map<std::string, serialize::ParameterPtr> databases_;
        if (!parameters[2]->children(databases_)) {
          result.add_error("unable to read metadata");
          return false;
        }

        std::string msg;

        if (!dba::exists::query(query_id, user_key, msg)) {
          result.add_error(Error::m(ERR_INVALID_QUERY_ID, query_id));
          return false;
        }

        if (!dba::exists::query(universe_query_id, user_key, msg)) {
          result.add_error(Error::m(ERR_INVALID_QUERY_ID, universe_query_id));
          return false;
        }

        const std::string norm_genome = utils::normalize_name(genome);
        if (!dba::exists::genome(norm_genome)) {
          result.add_error(Error::m(ERR_INVALID_GENOME_NAME, genome));
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
            if (!dba::exists::experiment(utils::normalize_name(experiment_name)) && !(utils::is_id(experiment_name, "q"))) {
              result.add_error(experiment_name + " is not a experiment name or valid query ID");
              return false;
            }
            experiments.push_back(experiment_name);
          }
          database_experiments[name] = experiments;
        }

        std::string request_id;
        if (!epidb::Engine::instance().queue_lola(query_id, universe_query_id, database_experiments, genome, user_key, request_id, msg)) {
          result.add_error(msg);
          return false;
        }

        result.add_string(request_id);
        return true;
      }

    } enrichRegionsOverlap;
  }
}
