//
//  find_similar.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 24.10.17.
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
#include "../dba/list.hpp"
#include "../dba/queries.hpp"

#include "../datatypes/user.hpp"

#include "../engine/commands.hpp"
#include "../engine/engine.hpp"

#include "../extras/utils.hpp"
#include "../extras/serialize.hpp"

#include "../errors.hpp"


namespace epidb {
  namespace command {

    class EnrichRegionsFastCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::ENRICHMENT, "Enrich the regions based on regions bitmap signature comparison.");
      }

      static Parameters parameters_()
      {
        return {
          parameters::QueryId,
          parameters::GenomeMultiple,
          Parameter("epigenetic_mark", serialize::STRING, "name(s) of selected epigenetic mark(s)", true),
          parameters::BioSourceMultiple,
          Parameter("sample", serialize::STRING, "id(s) of selected sample(s)", true),
          Parameter("technique", serialize::STRING, "name(s) of selected technique(s)", true),
          Parameter("project", serialize::STRING, "name(s) of selected projects", true),
          parameters::UserKey
        };
      }

      static Parameters results_()
      {
        return  {
          Parameter("request_id", serialize::STRING, "Request ID - Use it to retrieve the result with info() and get_request_data(). The result is a list containing the datasets that overlap with the query_id regions.")
        };
      }

    public:
      EnrichRegionsFastCommand() : Command("enrich_regions_fast", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        std::vector<serialize::ParameterPtr> genomes;
        std::vector<serialize::ParameterPtr> epigenetic_marks;
        std::vector<serialize::ParameterPtr> biosources;
        std::vector<serialize::ParameterPtr> sample_ids;
        std::vector<serialize::ParameterPtr> techniques;
        std::vector<serialize::ParameterPtr> projects;

        const std::string query_id = parameters[0]->as_string();
        parameters[1]->children(genomes);
        parameters[2]->children(epigenetic_marks);
        parameters[3]->children(biosources);
        parameters[4]->children(sample_ids);
        parameters[5]->children(techniques);
        parameters[6]->children(projects);

        const std::string user_key = parameters[7]->as_string();

        std::string msg;
        datatypes::User user;

        if (!check_permissions(user_key, datatypes::LIST_COLLECTIONS, user, msg )) {
          result.add_error(msg);
          return false;
        }

        std::vector<serialize::ParameterPtr> hardcoded_types = {
          serialize::ParameterPtr(new serialize::SimpleParameter(serialize::STRING, "peaks"))
        };

        mongo::BSONObj experiments_query;
        if (!dba::list::build_list_experiments_query(user, genomes, hardcoded_types,
            epigenetic_marks, biosources, sample_ids, techniques,
            projects, experiments_query, msg)) {
          result.add_error(msg);
          return false;
        }

        std::string request_id;
        if (!epidb::Engine::instance().queue_region_enrich_fast(user, query_id, experiments_query, request_id, msg)) {
          result.add_error(msg);
          return false;
        }
        result.add_string(request_id);

        return true;
      }
    } enrichRegionsFastCommand;
  }
}
