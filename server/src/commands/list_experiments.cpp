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

        mongo::BSONObjBuilder args_builder;

        if (!types.empty()) {
          args_builder.append("upload_info.content_format", utils::build_normalized_array(types));
        }

        if (!genomes.empty()) {
          args_builder.append("genome", utils::build_array(genomes));
          args_builder.append("norm_genome", utils::build_normalized_array(genomes));
        }


        if (!biosources.empty()) {
          args_builder.append("sample_info.biosource_name", utils::build_array(biosources));
          args_builder.append("sample_info.norm_biosource_name", utils::build_normalized_array(biosources));
        }

        // epigenetic mark
        if (!epigenetic_marks.empty()) {
          args_builder.append("epigenetic_mark", utils::build_array(epigenetic_marks));
          args_builder.append("norm_epigenetic_mark", utils::build_epigenetic_normalized_array(epigenetic_marks));
        }
        // sample id
        if (!sample_ids.empty()) {
          args_builder.append("sample_id", utils::build_array(sample_ids));
        }

        std::vector<utils::IdName> user_projects;
        if (!dba::list::projects(user_key, user_projects, msg)) {
          result.add_error(msg);
          return false;
        }

        // project.
        if (!projects.empty()) {

          // Filter the projects that are available to the user
          std::vector<serialize::ParameterPtr> filtered_projects;
          for (const auto& project : projects) {
            std::string norm_project = utils::normalize_name(project->as_string());
            bool found = false;
            for (const auto& user_project : user_projects) {
              std::string norm_user_project = utils::normalize_name(user_project.name);
              if (norm_project == norm_user_project) {
                filtered_projects.push_back(project);
                found = true;
                break;
              }
            }

            if (!found) {
              result.add_error("Project " + project->as_string() + " does not exists.");
              return false;
            }
          }
          args_builder.append("project", utils::build_array(filtered_projects));
          args_builder.append("norm_project", utils::build_normalized_array(filtered_projects));
        } else {

          std::vector<std::string> user_projects_names;
          for (const auto& project : user_projects) {
            user_projects_names.push_back(project.name);
          }

          args_builder.append("project", utils::build_array(user_projects_names));
          args_builder.append("norm_project", utils::build_normalized_array(user_projects_names));
        }

        // technique
        if (!techniques.empty()) {
          args_builder.append("technique", utils::build_array(techniques));
          args_builder.append("norm_technique", utils::build_normalized_array(techniques));
        }

        const mongo::BSONObj query = dba::query::build_query(args_builder.obj());
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
