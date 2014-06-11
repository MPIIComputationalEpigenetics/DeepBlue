//
//  select_regions.cpp
//  epidb
//
//  Created by Felipe Albrecht on 12.06.13.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include <sstream>
#include <map>
#include <set>

#include "../dba/dba.hpp"
#include "../dba/genomes.hpp"
#include "../dba/helpers.hpp"
#include "../dba/queries.hpp"

#include "../engine/commands.hpp"

#include "../extras/serialize.hpp"

namespace epidb {
    namespace command {

      class SelectRegionsCommand: public Command {

      private:static CommandDescription desc_() {
          return CommandDescription(categories::OPERATIONS, "Selects experiment regions matching the given parameters.");
        }

        static  Parameters parameters_() {
          Parameter p[] = {
            Parameter("experiment_name", serialize::STRING, "name(s) of selected experiment(s)", true),
            parameters::GenomeMultiple,
            Parameter("epigenetic_mark", serialize::STRING, "name(s) of selected epigenetic mark(s)", true),
            Parameter("sample", serialize::STRING, "id(s) of selected sample(s)", true),
            Parameter("technique", serialize::STRING, "name(s) of selected technique(es)", true),
            Parameter("project", serialize::STRING, "name(s) of selected projects", true),
            Parameter("chromosome", serialize::STRING, "chromosome name(s)", true),
            Parameter("start", serialize::INTEGER, "minimum start region"),
            Parameter("end", serialize::INTEGER, "maximum end region"),
            parameters::UserKey
          };
          Parameters params(&p[0], &p[0]+10);
          return params;
        }

        static Parameters results_() {
          Parameter p[] = {
            Parameter("id", serialize::STRING, "query id")
          };
          Parameters results(&p[0], &p[0]+1);
          return results;
        }

      public:
        SelectRegionsCommand() : Command("select_regions", parameters_(), results_(), desc_()) {}

        virtual bool run(const std::string& ip,
            const serialize::Parameters& parameters, serialize::Parameters& result) const
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

          const long long start = parameters[7]->isNull() ? -1 : parameters[7]->as_long();
          const long long end = parameters[8]->isNull() ? -1 : parameters[8]->as_long();

          std::string msg;
          bool ok;
          if (!dba::check_user(user_key, ok, msg)) {
            result.add_error(msg);
            return false;
          }
          if (!ok) {
            result.add_error("Invalid user key.");
            return false;
          }

          if (genomes.size() == 0) {
            result.add_error("Genome was not informed.");
            return false;
          }

          bool has_filter = false;
          mongo::BSONObjBuilder args_builder;

          if (experiment_names.size() > 0) {
            args_builder.append("experiment_name", dba::helpers::build_array(experiment_names));
            args_builder.append("norm_experiment_name", dba::helpers::build_normalized_array(experiment_names));
            has_filter = true;
          }
          // epigenetic mark
          if (epigenetic_marks.size() > 0) {
            args_builder.append("epigenetic_mark", dba::helpers::build_array(epigenetic_marks));
            args_builder.append("norm_epigenetic_mark", dba::helpers::build_epigenetic_normalized_array(epigenetic_marks));
            has_filter = true;
          }
          // sample id
          if (sample_ids.size() > 0) {
            args_builder.append("sample_id", dba::helpers::build_array(sample_ids));
            has_filter = true;
          }
          // project
          if (projects.size() > 0) {
            args_builder.append("project", dba::helpers::build_array(projects));
            args_builder.append("norm_project", dba::helpers::build_normalized_array(projects));
            has_filter = true;
          }
          // technique
          if (techniques.size() > 0) {
            args_builder.append("technique", dba::helpers::build_array(techniques));
            args_builder.append("norm_technique", dba::helpers::build_normalized_array(techniques));
            has_filter = true;
          }

          if (!has_filter) {
            result.add_error("At least one of the following fields must be provided: 'experiment name', 'epigenetic mark', 'sample ID', 'project', 'technique'.");
            return false;
          }

          args_builder.append("has_filter", has_filter);

          if (start >= 0) {
            args_builder.append("start", start);
          }
          if (end >= 0) {
            args_builder.append("end", end);
          }


          std::vector<std::string> genomes_s;
          std::vector<std::string> norm_genomes_s;
          std::vector<serialize::ParameterPtr>::iterator git;
          for (git = genomes.begin(); git != genomes.end(); ++git) {
            std::string genome = (*git)->as_string();
            std::string norm_genome = utils::normalize_name(genome);
            genomes_s.push_back(genome);
            norm_genomes_s.push_back(norm_genome);
          }

          std::set<std::string> chroms;
          if (chromosomes.size() == 0) {
            if (!dba::genomes::get_chromosomes(genomes_s, chroms, msg)) {
              result.add_error(msg);
              return false;
            }
          } else {
            std::vector<serialize::ParameterPtr>::iterator it;
            for (it = chromosomes.begin(); it != chromosomes.end(); ++it) {
              chroms.insert((**it).as_string());
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
