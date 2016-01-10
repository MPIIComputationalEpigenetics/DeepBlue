//
//  chromosomes.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 10.10.14.
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
#include "../dba/genomes.hpp"
#include "../dba/helpers.hpp"
#include "../datatypes/user.hpp"
#include "../engine/commands.hpp"

#include "../extras/utils.hpp"

#include "../errors.hpp"

namespace epidb {
  namespace command {

    class ChromosomesCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::GENOMES, "List all chromosomes of a given genome.");
      }

      static Parameters parameters_()
      {
        Parameter p[] = {
          parameters::Genome,
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 2);
        return params;
      }

      static Parameters results_()
      {
        Parameter p[] = {
          Parameter("chromosomes", serialize::LIST, "A list containing all chromosomes, with theirs names and sizes")
        };
        Parameters results(&p[0], &p[1]);
        return results;
      }

    public:
      ChromosomesCommand() : Command("chromosomes", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        const std::string genome = parameters[0]->as_string();
        const std::string user_key = parameters[1]->as_string();

        std::string msg;
        datatypes::User user;

        if (!check_permissions(user_key, datatypes::GET_DATA, user, msg )) {
          result.add_error(msg);
          return false;
        }

        std::vector<dba::genomes::ChromosomeInfo> chromosomes;
        if (!dba::genomes::get_chromosomes(genome, chromosomes, msg)) {
          result.add_error(msg);
          return false;
        }

        result.set_as_array(true);
        for (const dba::genomes::ChromosomeInfo & info : chromosomes) {
          std::vector<serialize::ParameterPtr> list;
          list.push_back(serialize::ParameterPtr(new serialize::SimpleParameter(info.name)));
          list.push_back(serialize::ParameterPtr(new serialize::SimpleParameter((long long) info.size)));
          result.add_list(list);
        }

        return true;
      }
    } chromosomesCommand;
  }
}
