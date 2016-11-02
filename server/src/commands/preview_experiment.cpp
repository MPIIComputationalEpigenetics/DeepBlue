//
//  preview_regions.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 29.06.16.
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
#include "../dba/experiments.hpp"
#include "../dba/exists.hpp"
#include "../dba/genomes.hpp"
#include "../dba/queries.hpp"
#include "../dba/list.hpp"
#include "../dba/retrieve.hpp"

#include "../datatypes/user.hpp"

#include "../extras/utils.hpp"
#include "../extras/serialize.hpp"

#include "../processing/processing.hpp"

#include "../errors.hpp"

namespace epidb {
  namespace command {

    class PreviewExperimentCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::EXPERIMENTS, " List the DeepBlue Experiments that matches the search criteria defined by this command parameters.");
      }

      static Parameters parameters_()
      {
        Parameter p[] = {
          Parameter("experiment_name", serialize::STRING, "name(s) of selected experiment(s)"),
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 2);
        return params;
      }

      static Parameters results_()
      {
        Parameter p[] = {
          Parameter("experiment", serialize::STRING, "experiment's regions")
        };
        Parameters results(&p[0], &p[0] + 1);
        return results;
      }

    public:
      PreviewExperimentCommand() : Command("preview_experiment", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        const std::string experiment_name_or_id = parameters[0]->as_string();
        const std::string user_key = parameters[1]->as_string();

        std::string msg;
        datatypes::User user;

        if (!check_permissions(user_key, datatypes::GET_DATA, user, msg )) {
          result.add_error(msg);
          return false;
        }

        std::string experiment_name;
        std::string experiment_norm_name;

        if (!dba::experiments::get_experiment_name(experiment_name_or_id, experiment_name, experiment_norm_name, msg)) {
          result.add_error(msg);
          return false;
        }


        if (!dba::exists::experiment(experiment_norm_name)) {
          result.add_error(Error::m(ERR_INVALID_EXPERIMENT, experiment_name));
          return false;
        }


        std::vector<utils::IdName> user_projects;
        if (!dba::list::projects(user_key, user_projects, msg)) {
          result.add_error(msg);
          return false;
        }
        std::vector<std::string> user_projects_names;
        for (const auto& project : user_projects) {
          user_projects_names.push_back(project.name);
        }

        std::string genome;
        if (!dba::experiments::get_genome(experiment_norm_name, genome, msg)) {
          return false;
        }
        std::string norm_genome = utils::normalize_name(genome);

        std::set<std::string> chroms;
        if (!dba::genomes::get_chromosomes(norm_genome, chroms, msg)) {
          result.add_error(msg);
          return false;
        }

        mongo::BSONObjBuilder args_builder;
        args_builder.append("experiment_name", experiment_name);
        args_builder.append("norm_experiment_name", experiment_norm_name);
        args_builder.append("project", utils::build_array(user_projects_names));
        args_builder.append("norm_project", utils::build_normalized_array(user_projects_names));
        args_builder.append("chromosomes", chroms);
        args_builder.append("genomes", genome);
        args_builder.append("norm_genomes", norm_genome);

        mongo::BSONObj args = args_builder.obj();
        std::cerr << args.toString() << std::endl;

        mongo::BSONObj regions_query;
        if (!dba::query::build_experiment_query(args, regions_query, msg)) {
          result.add_error(msg);
          return false;
        }

        std::cerr << regions_query.toString() << std::endl;

        std::vector<std::string> chroms_vector;
        std::copy(chroms.begin(), chroms.end(), std::back_inserter(chroms_vector));

        ChromosomeRegionsList chromosomeRegionsList;
        size_t chrom_pos = 0;
        do {
          Regions regions;
          if (!dba::retrieve::get_regions_preview(norm_genome, chroms_vector[chrom_pos], regions_query, regions, msg)) {
            result.add_error(msg);
            return false;
          }
          if (!regions.empty()) {
            Regions preview_regions;
            size_t count = std::min(regions.size(), static_cast<size_t>(5));
            preview_regions.insert(preview_regions.end(), std::make_move_iterator(regions.begin()), std::make_move_iterator(regions.begin() + count));
            chromosomeRegionsList.emplace_back(chroms_vector[chrom_pos], std::move(preview_regions));
          }
        } while(chromosomeRegionsList.empty() && ++chrom_pos < chroms_vector.size());

        processing::StatusPtr status = processing::build_dummy_status();

        mongo::BSONObj experiment_bson;
        if (!dba::experiments::by_name(experiment_norm_name, experiment_bson, msg)) {
          result.add_error(msg);
          return false;
        }

        std::string experiment_format = experiment_bson["format"].str();

        StringBuilder sb;
        if (!epidb::processing::format_regions(experiment_format, chromosomeRegionsList, status, sb, msg)) {
          return false;
        }

        std::replace( experiment_format.begin(), experiment_format.end(), ',', '\t');

        std::string output = experiment_format + "\n" + sb.to_string();

        result.add_string(output);
        return true;
      }
    } previewExperimentCommand;
  }
}
