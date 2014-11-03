//
//  list_experiments.cpp
//  epidb
//
//  Created by Felipe Albrecht on 25.06.13.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include <boost/foreach.hpp>

#include "../engine/commands.hpp"

#include "../dba/dba.hpp"
#include "../dba/helpers.hpp"
#include "../dba/list.hpp"
#include "../dba/queries.hpp"

#include "../extras/utils.hpp"
#include "../extras/serialize.hpp"

#include "../errors.hpp"

namespace epidb {
  namespace command {

    class ListExperimentsCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::EXPERIMENTS, "Lists all existing experiments.");
      }

      static Parameters parameters_()
      {
        Parameter p[] = {
          parameters::GenomeMultiple,
          Parameter("epigenetic_mark", serialize::STRING, "name(s) of selected epigenetic mark(s)", true),
          Parameter("sample", serialize::STRING, "id(s) of selected sample(s)", true),
          Parameter("technique", serialize::STRING, "name(s) of selected technique(es)", true),
          Parameter("project", serialize::STRING, "name(s) of selected projects", true),
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 6);
        return params;
      }

      static Parameters results_()
      {
        Parameter p[] = {
          Parameter("experiments", serialize::LIST, "experiment names")
        };
        Parameters results(&p[0], &p[0] + 1);
        return results;
      }

    public:
      ListExperimentsCommand() : Command("list_experiments", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        std::vector<serialize::ParameterPtr> genomes;
        std::vector<serialize::ParameterPtr> epigenetic_marks;
        std::vector<serialize::ParameterPtr> sample_ids;
        std::vector<serialize::ParameterPtr> techniques;
        std::vector<serialize::ParameterPtr> projects;

        parameters[0]->children(genomes);
        parameters[1]->children(epigenetic_marks);
        parameters[2]->children(sample_ids);
        parameters[3]->children(techniques);
        parameters[4]->children(projects);

        const std::string user_key = parameters[5]->as_string();

        std::string msg;
        if (!Command::checks(user_key, msg)) {
          result.add_error(msg);
          return false;
        }

        mongo::BSONObjBuilder args_builder;
        if (!genomes.empty()) {
          args_builder.append("genome", dba::helpers::build_array(genomes));
          args_builder.append("norm_genome", dba::helpers::build_normalized_array(genomes));
        }
        // epigenetic mark
        if (!epigenetic_marks.empty()) {
          args_builder.append("epigenetic_mark", dba::helpers::build_array(epigenetic_marks));
          args_builder.append("norm_epigenetic_mark", dba::helpers::build_epigenetic_normalized_array(epigenetic_marks));
        }
        // sample id
        if (!sample_ids.empty()) {
          args_builder.append("sample_id", dba::helpers::build_array(sample_ids));
        }
        // project
        if (!projects.empty()) {
          args_builder.append("project", dba::helpers::build_array(projects));
          args_builder.append("norm_project", dba::helpers::build_normalized_array(projects));
        }
        // technique
        if (!techniques.empty()) {
          args_builder.append("technique", dba::helpers::build_array(techniques));
          args_builder.append("norm_technique", dba::helpers::build_normalized_array(techniques));
        }

        const mongo::BSONObj query = dba::query::build_query(args_builder.obj());
        std::vector<utils::IdName> names;
        if (!dba::list::experiments(mongo::Query(query), names, msg)) {
          result.add_error(msg);
        }

        set_id_names_return(names, result);

        return true;
      }
    } listExperimentsCommand;
  }
}
