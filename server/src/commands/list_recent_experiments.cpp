//
//  dummy.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 30.07.13.
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

#include <mongo/bson/bson.h>

#include "../engine/commands.hpp"

#include "../dba/dba.hpp"
#include "../dba/queries.hpp"
#include "../dba/list.hpp"

#include "../datatypes/user.hpp"

#include "../extras/serialize.hpp"
#include "../extras/utils.hpp"
#include "../errors.hpp"

namespace epidb {
  namespace command {

    class ListRecentExperiments: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::EXPERIMENTS, " List the latest Experiments included in DeepBlue that match the defined criteria informed in the parameters.");
      }

      static Parameters parameters_()
      {
        Parameter p[] = {
          Parameter("days", serialize::DOUBLE, "maximum days ago the experiments were added"),
          parameters::GenomeMultiple,
          Parameter("epigenetic_mark", serialize::STRING, "name(s) of selected epigenetic mark(s)", true),
          Parameter("sample", serialize::STRING, "id(s) of selected sample(s)", true),
          Parameter("technique", serialize::STRING, "name(s) of selected technique(es)", true),
          Parameter("project", serialize::STRING, "name(s) of selected projects", true),
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 7);
        return params;
      }

      static Parameters results_()
      {
        Parameter p[] = {
          Parameter("experiments", serialize::LIST, "names of recent experiments")
        };
        Parameters results(&p[0], &p[0] + 1);
        return results;
      }

    public:
      ListRecentExperiments() : Command("list_recent_experiments", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        double days = parameters[0]->as_number();
        const std::string user_key = parameters[6]->as_string();

        std::vector<serialize::ParameterPtr> genomes;
        std::vector<serialize::ParameterPtr> epigenetic_marks;
        std::vector<serialize::ParameterPtr> sample_ids;
        std::vector<serialize::ParameterPtr> techniques;
        std::vector<serialize::ParameterPtr> projects;

        parameters[1]->children(genomes);
        parameters[2]->children(epigenetic_marks);
        parameters[3]->children(sample_ids);
        parameters[4]->children(techniques);
        parameters[5]->children(projects);

        std::string msg;
        datatypes::User user;

        if (!check_permissions(user_key, datatypes::LIST_COLLECTIONS, user, msg )) {
          result.add_error(msg);
          return false;
        }

        mongo::BSONObjBuilder args_builder;
        if (genomes.size() > 0) {
          args_builder.append("genome", utils::build_array(genomes));
          args_builder.append("norm_genome", utils::build_normalized_array(genomes));
        }
        // epigenetic mark
        if (epigenetic_marks.size() > 0) {
          args_builder.append("epigenetic_mark", utils::build_array(epigenetic_marks));
          args_builder.append("norm_epigenetic_mark", utils::build_epigenetic_normalized_array(epigenetic_marks));
        }
        // sample id
        if (sample_ids.size() > 0) {
          args_builder.append("sample_id", utils::build_array(sample_ids));
        }
        // project
        if (projects.size() > 0) {
          args_builder.append("project", utils::build_array(projects));
          args_builder.append("norm_project", utils::build_normalized_array(projects));
        }
        // technique
        if (techniques.size() > 0) {
          args_builder.append("technique", utils::build_array(techniques));
          args_builder.append("norm_technique", utils::build_normalized_array(techniques));
        }

        time_t time_;
        time(&time_);
        mongo::Date_t after_date(time_ - days * 24 * 60 * 60);
        mongo::BSONObjBuilder sub(args_builder.subobjStart("upload_info.upload_end"));
        sub.appendTimeT("$gt", after_date);
        sub.done();

        mongo::BSONObj o = args_builder.obj();
        const mongo::BSONObj q = dba::query::build_query(o);
        mongo::Query query = mongo::Query(q).sort("upload_info.upload_end", -1);

        std::vector<utils::IdName> names;
        if (!dba::list::experiments(query, names, msg)) {
          result.add_error(msg);
        }

        set_id_names_return(names, result);

        return true;
      }

    } listRecentExperiments;
  }
}



