//
//  tiling_regions.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Fabian Reinartz on 05.03.2014
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

#include <set>

#include "../dba/dba.hpp"
#include "../dba/exists.hpp"
#include "../dba/genomes.hpp"
#include "../dba/helpers.hpp"
#include "../dba/queries.hpp"
#include "../datatypes/user.hpp"

#include "../extras/serialize.hpp"

#include "../engine/commands.hpp"

#include "../errors.hpp"

namespace epidb {
  namespace command {

    class TilingRegionsCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::OPERATIONS, "Creates regions with the tiling size over the chromosomes.");
      }

      static  Parameters parameters_()
      {
        Parameter p[] = {
          Parameter("size", serialize::INTEGER, "tiling size"),
          parameters::Genome,
          Parameter("chromosome", serialize::STRING, "chromosome name(s)", true),
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 4);
        return params;
      }

      static Parameters results_()
      {
        Parameter p[] = {
          Parameter("id", serialize::STRING, "query id")
        };
        Parameters results(&p[0], &p[0] + 1);
        return results;
      }

    public:
      TilingRegionsCommand() : Command("tiling_regions", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        std::vector<serialize::ParameterPtr> chromosomes;
        parameters[2]->children(chromosomes);

        const unsigned int size = parameters[0]->as_long();
        std::string genome = parameters[1]->as_string();
        const std::string user_key = parameters[3]->as_string();

        std::string msg;
        datatypes::User user;

        if (!check_permissions(user_key, datatypes::GET_DATA, user, msg )) {
          result.add_error(msg);
          return false;
        }

        std::string norm_genome = utils::normalize_name(genome);
        if (!dba::exists::genome(norm_genome)) {
          result.add_error("Invalid genome '" + genome + "'");
          return false;
        }

        mongo::BSONObjBuilder args_builder;
        args_builder.append("genome", genome);
        args_builder.append("norm_genome", norm_genome);
        args_builder.append("size", size);

        // if no chromosomes were provided, default to all chromosomes of the genome
        std::set<std::string> chroms;
        if (chromosomes.size() == 0) {
          if (!dba::genomes::get_chromosomes(norm_genome, chroms, msg)) {
            result.add_error(msg);
            return false;
          }
        } else {
          std::vector<serialize::ParameterPtr>::iterator it;
          for (it = chromosomes.begin(); it != chromosomes.end(); ++it) {
            chroms.insert((**it).as_string());
          }
        }
        args_builder.append("chromosomes", chroms);

        std::string query_id;
        if (!dba::query::store_query("tiling", args_builder.obj(), user_key, query_id, msg)) {
          result.add_error(msg);
          return false;
        }

        result.add_string(query_id);
        return true;
      }

    } tilingRegionsCommand;
  }
}
