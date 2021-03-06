//
//  collection_experiments_count.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 02.05.16.
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

#include <future>
#include <string>
#include <thread>
#include <tuple>

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

    class CollectionExperimentsCountCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::EXPERIMENTS, "Count the number of experiments that match the selection criteria in each term of the selected controlled_vocabulary. The selection can be achieved through specifying a list of BioSources, experimental Techniques, Epigenetic Marks, Samples or Projects.");
      }

      static Parameters parameters_()
      {
        Parameter p[] = {
          Parameter("controlled_vocabulary", serialize::STRING, "controlled vocabulary name"),
          parameters::GenomeMultiple,
          Parameter("type", serialize::STRING, "type of the experiment: peaks or signal", true),
          Parameter("epigenetic_mark", serialize::STRING, "name(s) of selected epigenetic mark(s)", true),
          parameters::BioSourceMultiple,
          Parameter("sample", serialize::STRING, "id(s) of selected sample(s)", true),
          Parameter("technique", serialize::STRING, "name(s) of selected technique(s)", true),
          Parameter("project", serialize::STRING, "name(s) of selected projects", true),
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 9);
        return params;
      }

      static Parameters results_()
      {
        Parameter p[] = {
          Parameter("terms", serialize::LIST, "controlled_vocabulary terms with count")
        };
        Parameters results(&p[0], &p[0] + 1);
        return results;
      }

    public:
      CollectionExperimentsCountCommand() : Command("collection_experiments_count", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        const std::string controlled_vocabulary = parameters[0]->as_string();

        std::vector<serialize::ParameterPtr> genomes;
        std::vector<serialize::ParameterPtr> types;
        std::vector<serialize::ParameterPtr> epigenetic_marks;
        std::vector<serialize::ParameterPtr> biosources;
        std::vector<serialize::ParameterPtr> sample_ids;
        std::vector<serialize::ParameterPtr> techniques;
        std::vector<serialize::ParameterPtr> projects;

        parameters[1]->children(genomes);
        parameters[2]->children(types);
        parameters[3]->children(epigenetic_marks);
        parameters[4]->children(biosources);
        parameters[5]->children(sample_ids);
        parameters[6]->children(techniques);
        parameters[7]->children(projects);

        const std::string user_key = parameters[8]->as_string();

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

        std::vector<utils::IdNameCount> experiments_count;
        if (!dba::list::collection_experiments_count(user, controlled_vocabulary, query, experiments_count, msg)) {
          result.add_error(msg);
        }

        result.set_as_array(true);

        for (const auto& value : experiments_count) {
          serialize::ParameterPtr item = std::make_shared<serialize::ListParameter>();

          item->add_child(serialize::ParameterPtr(new serialize::SimpleParameter(value.id)));
          item->add_child(serialize::ParameterPtr(new serialize::SimpleParameter(value.name)));
          item->add_child(serialize::ParameterPtr(new serialize::SimpleParameter(static_cast<long long>(value.count))));

          result.add_param(item);
        }

        return true;
      }
    } collectionExperimentsCountCommand;
  }
}
