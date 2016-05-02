//
//  faceting_experiments.cpp
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

    class FacetingExperimentsCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::EXPERIMENTS, "Experiments faceting.");
      }

      static Parameters parameters_()
      {
        Parameter p[] = {
          parameters::GenomeMultiple,
          Parameter("type", serialize::STRING, "type of the experiment: peaks or signal", true),
          Parameter("epigenetic_mark", serialize::STRING, "name(s) of selected epigenetic mark(s)", true),
          Parameter("biosource", serialize::STRING, "name(s) of selected biosource(s)", true),
          Parameter("sample", serialize::STRING, "id(s) of selected sample(s)", true),
          Parameter("technique", serialize::STRING, "name(s) of selected technique(s)", true),
          Parameter("project", serialize::STRING, "name(s) of selected projects", true),
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 8);
        return params;
      }

      static Parameters results_()
      {
        Parameter p[] = {
          Parameter("faceting", serialize::MAP, "Map with the mandatory fields of the experiments metadata, where each contains a list of terms that appears.")
        };
        Parameters results(&p[0], &p[0] + 1);
        return results;
      }

      static std::tuple<std::string, serialize::ParameterPtr> __build_face_result(const std::string& key, const std::vector<utils::IdNameCount>& values)
      {
        serialize::ParameterPtr face_values = std::make_shared<serialize::ListParameter>();

        for (const auto& value : values) {
          serialize::ParameterPtr item = std::make_shared<serialize::ListParameter>();

          item->add_child(serialize::ParameterPtr(new serialize::SimpleParameter(value.id)));
          item->add_child(serialize::ParameterPtr(new serialize::SimpleParameter(value.name)));
          item->add_child(serialize::ParameterPtr(new serialize::SimpleParameter(static_cast<long long>(value.count))));

          face_values->add_child(item);
        }

        return std::make_tuple(key, face_values);
      }

    public:
      FacetingExperimentsCommand() : Command("faceting_experiments", parameters_(), results_(), desc_()) {}

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

        if (!dba::list::build_list_experiments_query(genomes, types, epigenetic_marks, biosources, sample_ids, techniques,
            projects, user_key, query, msg)) {
          result.add_error(msg);
          return false;
        }

        std::unordered_map<std::string, std::vector<utils::IdNameCount>> faceting_result;

        if (!dba::list::faceting(query, user_key, faceting_result, msg)) {
          result.add_error(msg);
        }

        serialize::ParameterPtr faces(new serialize::MapParameter());

        std::vector<std::future<std::tuple<std::string, serialize::ParameterPtr> > > threads;
        for (const auto& face : faceting_result) {
          const auto& key = face.first;
          const auto& values = face.second;
          auto t = std::async(std::launch::async, &__build_face_result, std::ref(key), std::ref(values));
          threads.push_back(std::move(t));
        }

        size_t thread_count = threads.size();
        for (size_t i = 0; i < thread_count; ++i) {
          threads[i].wait();
          auto result = threads[i].get();
          faces->add_child(std::get<0>(result), std::get<1>(result));
        }

        result.add_param(faces);

        return true;
      }
    } facetingExperimentsCommand;
  }
}
