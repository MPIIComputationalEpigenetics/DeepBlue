//
//  enrich_regions.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 02.03.17.
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

    class EnrichRegionsGoTermsCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::ENRICHMENT, "Enrich the regions based on Gene Ontology terms.");
      }

      static  Parameters parameters_()
      {
        return {
          parameters::QueryId,
          parameters::GeneModel,
          parameters::UserKey
        };
      }

      static Parameters results_()
      {
        return {
          Parameter("request_id", serialize::STRING, "Request ID - Use it to retrieve the result with info() and get_request_data(). The result is a list containing the GO terms that overlap with the query_id regions.")
        };
      }

    public:
      EnrichRegionsGoTermsCommand() : Command("enrich_regions_go_terms", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        const std::string query_id = parameters[0]->as_string();
        const std::string gene_model = parameters[1]->as_string();
        const std::string user_key = parameters[2]->as_string();

        std::string msg;
        datatypes::User user;

        if (!check_permissions(user_key, datatypes::GET_DATA, user, msg )) {
          result.add_error(msg);
          return false;
        }

        if (!dba::exists::query(query_id, user_key, msg)) {
          result.add_error(Error::m(ERR_INVALID_QUERY_ID, query_id));
          return false;
        }

        std::string request_id;
        if (!epidb::Engine::instance()
            .queue_calculate_enrichment(query_id, gene_model, user_key, request_id, msg)) {
          result.add_error(msg);
          return false;
        }

        result.add_string(request_id);
        return true;
      }
    } enrichRegionsGoTermsCommand;
  }
}
