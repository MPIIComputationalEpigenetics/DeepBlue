//
//  dummy.cpp
//  epidb
//
//  Created by Felipe Albrecht on 30.07.13.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include <mongo/bson/bson.h>

#include "../engine/commands.hpp"

#include "../dba/dba.hpp"
#include "../dba/queries.hpp"
#include "../dba/helpers.hpp"
#include "../dba/list.hpp"

#include "../datatypes/user.hpp"

#include "../extras/serialize.hpp"
#include "../errors.hpp"

namespace epidb {
  namespace command {

    class ListRecentExperiments: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::EXPERIMENTS, "Lists all recent experiments.");
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
          args_builder.append("genome", dba::helpers::build_array(genomes));
          args_builder.append("norm_genome", dba::helpers::build_normalized_array(genomes));
        }
        // epigenetic mark
        if (epigenetic_marks.size() > 0) {
          args_builder.append("epigenetic_mark", dba::helpers::build_array(epigenetic_marks));
          args_builder.append("norm_epigenetic_mark", dba::helpers::build_epigenetic_normalized_array(epigenetic_marks));
        }
        // sample id
        if (sample_ids.size() > 0) {
          args_builder.append("sample_id", dba::helpers::build_array(sample_ids));
        }
        // project
        if (projects.size() > 0) {
          args_builder.append("project", dba::helpers::build_array(projects));
          args_builder.append("norm_project", dba::helpers::build_normalized_array(projects));
        }
        // technique
        if (techniques.size() > 0) {
          args_builder.append("technique", dba::helpers::build_array(techniques));
          args_builder.append("norm_technique", dba::helpers::build_normalized_array(techniques));
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



