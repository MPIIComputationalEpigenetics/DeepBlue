//
//  select_regions.cpp
//  DeepBlue Epigenomic Data Server

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
//  Created by Felipe Albrecht on 12.06.13.
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

#include <sstream>
#include <map>
#include <set>

#include "../dba/dba.hpp"
#include "../dba/experiments.hpp"
#include "../dba/exists.hpp"
#include "../dba/genomes.hpp"
#include "../dba/helpers.hpp"
#include "../dba/list.hpp"
#include "../dba/queries.hpp"

#include "../datatypes/user.hpp"

#include "../engine/commands.hpp"

#include "../extras/serialize.hpp"

#include "../errors.hpp"

namespace epidb {
  namespace command {

    class SelectRegionsCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::OPERATIONS, "Selects experiment regions matching the given parameters.");
      }

      static  Parameters parameters_()
      {
        Parameter p[] = {
          Parameter("experiment_name", serialize::STRING, "name(s) of selected experiment(s)", true),
          parameters::GenomeMultiple,
          Parameter("epigenetic_mark", serialize::STRING, "name(s) of selected epigenetic mark(s)", true),
          Parameter("sample_id", serialize::STRING, "id(s) of selected sample(s)", true),
          Parameter("technique", serialize::STRING, "name(s) of selected technique(es)", true),
          Parameter("project", serialize::STRING, "name(s) of selected projects", true),
          Parameter("chromosome", serialize::STRING, "chromosome name(s)", true),
          Parameter("start", serialize::INTEGER, "minimum start region"),
          Parameter("end", serialize::INTEGER, "maximum end region"),
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 10);
        return params;
      }

      static Parameters results_()
      {
        Parameter p[] = {
          Parameter("id", serialize::STRING, "query id")
        };
        Parameters results(&p[0], &p[0] + 1);
        return results;
      }

    public:
      SelectRegionsCommand() : Command("select_regions", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        std::vector<serialize::ParameterPtr> experiment_names;
        std::vector<serialize::ParameterPtr> genomes;
        std::vector<serialize::ParameterPtr> epigenetic_marks;
        std::vector<serialize::ParameterPtr> sample_ids;
        std::vector<serialize::ParameterPtr> techniques;
        std::vector<serialize::ParameterPtr> projects;
        std::vector<serialize::ParameterPtr> chromosomes;

        parameters[0]->children(experiment_names);
        parameters[1]->children(genomes);
        parameters[2]->children(epigenetic_marks);
        parameters[3]->children(sample_ids);
        parameters[4]->children(techniques);
        parameters[5]->children(projects);
        parameters[6]->children(chromosomes);

        const std::string user_key = parameters[9]->as_string();

        const int start = parameters[7]->isNull() ? -1 : parameters[7]->as_long();
        const int end = parameters[8]->isNull() ? -1 : parameters[8]->as_long();

        std::string msg;
        datatypes::User user;

        if (!check_permissions(user_key, datatypes::GET_DATA, user, msg )) {
          result.add_error(msg);
          return false;
        }

        bool has_exp_or_genome = false;
        mongo::BSONObjBuilder args_builder;

        std::vector<std::string> names;
        std::vector<std::string> norm_names;
        if (experiment_names.size() > 0) {
          std::vector<std::string> exp_names_string = utils::build_vector(experiment_names);
          if (!dba::experiments::get_experiments_names(exp_names_string, names, norm_names, msg)) {
            result.add_error(msg);
            return false;
          }
          args_builder.append("experiment_name", utils::build_array(names));
          args_builder.append("norm_experiment_name", utils::build_array(norm_names));

          if (!utils::check_parameters(names, utils::normalize_name, dba::exists::experiment, msg)) {
            result.add_error("Experiment " + msg + " does not exists.");
            return false;
          }
          has_exp_or_genome = true;
        }
        // epigenetic mark
        if (epigenetic_marks.size() > 0) {
          if (!utils::check_parameters(epigenetic_marks, utils::normalize_epigenetic_mark, dba::exists::epigenetic_mark, msg)) {
            result.add_error("Epigenetic mark " + msg + " does not exists.");
            return false;
          }
          args_builder.append("epigenetic_mark", utils::build_array(epigenetic_marks));
          args_builder.append("norm_epigenetic_mark", utils::build_epigenetic_normalized_array(epigenetic_marks));
        }
        // sample id
        if (sample_ids.size() > 0) {
          if (!utils::check_parameters(sample_ids, utils::normalize_name, dba::exists::sample, msg)) {
            result.add_error("Sample ID " + msg + " does not exists.");
            return false;
          }
          args_builder.append("sample_id", utils::build_array(sample_ids));
        }

        std::vector<utils::IdName> user_projects;
        if (!dba::list::projects(user_key, user_projects, msg)) {
          result.add_error(msg);
          return false;
        }

        // project
        if (projects.size() > 0) {
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
        if (techniques.size() > 0) {
          if (!utils::check_parameters(techniques, utils::normalize_name, dba::exists::technique, msg)) {
            result.add_error("Technique " + msg + " does not exists.");
            return false;
          }
          args_builder.append("technique", utils::build_array(techniques));
          args_builder.append("norm_technique", utils::build_normalized_array(techniques));
        }

        if (start >= 0) {
          args_builder.append("start", (int) start);
        }
        if (end >= 0) {
          args_builder.append("end", (int) end);
        }


        std::set<std::string> genomes_s;
        std::set<std::string> norm_genomes_s;

        std::vector<serialize::ParameterPtr>::iterator git;
        for (git = genomes.begin(); git != genomes.end(); ++git) {
          std::string genome = (*git)->as_string();
          std::string norm_genome = utils::normalize_name(genome);
          genomes_s.insert(genome);
          norm_genomes_s.insert(norm_genome);
          has_exp_or_genome = true;
        }

        if (!has_exp_or_genome) {
          result.add_error("At least one experiment_name or one genome must be informed.");
          return false;
        }

        // We have to load the genomes from the experiments if the user did not specify the genome in the query
        if (genomes_s.empty()) {
          for (auto experiment_norm_name : norm_names) {
            std::string genome;
            if (!dba::experiments::get_genome(experiment_norm_name, genome, msg)) {
              return false;
            }
            genomes_s.insert(genome);
            norm_genomes_s.insert(genome);
          }
        }

        std::set<std::string> chroms;
        if (chromosomes.size() == 0) {
          if (!dba::genomes::get_chromosomes(genomes_s, chroms, msg)) {
            result.add_error(msg);
            return false;
          }
        } else {
          for (auto parameter_ptr : chromosomes) {
            chroms.insert(parameter_ptr->as_string());
          }
        }
        args_builder.append("chromosomes", chroms);
        args_builder.append("genomes", genomes_s);
        args_builder.append("norm_genomes", norm_genomes_s);

        std::string query_id;
        if (!dba::query::store_query("experiment_select", args_builder.obj(), user_key, query_id, msg)) {
          result.add_error(msg);
          return false;
        }

        result.add_string(query_id);
        return true;
      }

    } selectRegionsCommand;
  }
}
