//
//  list_experiments.cpp
//  epidb
//
//  Created by Felipe Albrecht on 25.06.13.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
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
        return CommandDescription(categories::EXPERIMENTS, "Lists all existing experiments.");
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

        std::vector<utils::IdName> names;
        if (!dba::list::experiments(query, names, msg)) {
          result.add_error(msg);
        }

        set_id_names_return(names, result);

        return true;
      }
    } listExperimentsCommand;
  }
}
