//
//  list_experiments.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 25.06.13.
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

#include "../engine/commands.hpp"

#include "../dba/dba.hpp"
#include "../dba/list.hpp"
#include "../dba/queries.hpp"

#include "../datatypes/user.hpp"

#include "../extras/utils.hpp"
#include "../extras/serialize.hpp"

#include "../errors.hpp"

namespace epidb {
  namespace command {

    class ListExperimentsCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::EXPERIMENTS, " List the DeepBlue Experiments that matches the search criteria defined by this command parameters.");
      }

      static Parameters parameters_()
      {
        return {
          parameters::GenomeMultiple,
          Parameter("type", serialize::STRING, "type of the experiment: peaks or signal", true),
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
        return {
          Parameter("experiments", serialize::LIST, "experiment names and IDS")
        };
      }

    public:
      ListExperimentsCommand() : Command("list_experiments", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        std::vector<serialize::ParameterPtr> genomes;
        std::vector<serialize::ParameterPtr> types;
        std::vector<serialize::ParameterPtr> epigenetic_marks;
        std::vector<serialize::ParameterPtr> biosources;
        std::vector<serialize::ParameterPtr> sample_ids;
        std::vector<serialize::ParameterPtr> techniques;
        std::vector<serialize::ParameterPtr> projects;

        parameters[0]->children(genomes);
        parameters[1]->children(types);
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

        mongo::BSONObj query;

        if (!dba::list::build_list_experiments_query(user, genomes, types, epigenetic_marks, biosources, sample_ids, techniques,
                                          projects, query, msg)) {
          result.add_error(msg);
          return false;
        }

        std::vector<utils::IdName> names;
        if (!dba::list::experiments(query, names, msg)) {
          result.add_error(msg);
          return false;
        }

        set_id_names_return(names, result);

        return true;
      }
    } listExperimentsCommand;
  }
}
