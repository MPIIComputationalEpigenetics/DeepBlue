//
//  find_motif.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 06.05.14. (find_pattern.cpp)
//  Renamed to find_motif on 01.06.17.
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

#include <string>
#include <sstream>

#include "../datatypes/user.hpp"

#include "../dba/dba.hpp"
#include "../dba/genomes.hpp"
#include "../dba/queries.hpp"

#include "../engine/commands.hpp"

#include "../extras/utils.hpp"

#include "../errors.hpp"

namespace epidb {
  namespace command {

    class FindMotifCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::ANNOTATIONS, "Find genomic regions based on a given motif that appears in the genomic sequence.");
      }

      static Parameters parameters_()
      {
        return {
          Parameter("motif", serialize::STRING, "motif (PERL regular expression)"),
          parameters::Genome,
          parameters::ChromosomeMultiple,
          Parameter("start", serialize::INTEGER, "minimum start region"),
          Parameter("end", serialize::INTEGER, "maximum end region"),
          Parameter("overlap", serialize::BOOLEAN, "if the matching should do overlap search"),
          parameters::UserKey
        };
      }

      static Parameters results_()
      {
        return {
          Parameter("id", serialize::STRING, "id of the annotation that contains the positions of the given motif")
        };
      }

    public:
      FindMotifCommand() : Command("find_motif", parameters_(), results_(), desc_()) {}

      // TODO: Check user
      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        const std::string motif = parameters[0]->as_string();
        const std::string genome = parameters[1]->as_string();
        const int start = parameters[3]->isNull() ? -1 : parameters[3]->as_long();
        const int end = parameters[4]->isNull() ? -1 : parameters[4]->as_long();
        bool overlap = parameters[5]->as_boolean();
        const std::string user_key = parameters[6]->as_string();

        std::vector<serialize::ParameterPtr> chromosomes;
        parameters[2]->children(chromosomes);

        std::string msg;
        datatypes::User user;

        if (!check_permissions(user_key, datatypes::GET_DATA, user, msg )) {
          result.add_error(msg);
          return false;
        }

        if (motif.empty()) {
          result.add_error(Error::m(ERR_USER_MOTIF_MISSING));
          return false;
        }

        if (genome.empty()) {
          result.add_error(Error::m(ERR_USER_GENOME_MISSING));
          return false;
        }

        const std::string norm_genome = utils::normalize_name(genome);

        mongo::BSONObjBuilder args_builder;

        args_builder.append("motif", motif);
        args_builder.append("genome", genome);
        args_builder.append("norm_genome", norm_genome);

        std::set<std::string> chroms;
        if (!chromosomes.empty()) {
          for (auto parameter_ptr : chromosomes) {
            chroms.insert(parameter_ptr->as_string());
          }
          args_builder.append("chromosomes", chroms);
        }

        if (start > 0) {
          args_builder.append("start", (int) start);
        } else {
          args_builder.append("start", -1);
        }

        if (end > 0) {
          args_builder.append("end", (int) end);
        } else {
          args_builder.append("end", -1);
        }

        args_builder.append("overlap", overlap);

        std::string query_id;
        if (!dba::query::store_query("find_motif", args_builder.obj(), user_key, query_id, msg)) {
          result.add_error(msg);
          return false;
        }

        result.add_string(query_id);
        return true;
      }

    } findMotifCommand;

  }
}
