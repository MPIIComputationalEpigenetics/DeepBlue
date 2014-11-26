//
//  score_matrix.cpp
//  epidb
//
//  Created by Felipe Albrecht on 08.11.14.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include <map>
#include <sstream>
#include <vector>

#include "../algorithms/accumulator.hpp"
#include "../dba/helpers.hpp"
#include "../dba/queries.hpp"
#include "../dba/experiments.hpp"
#include "../engine/commands.hpp"
#include "../extras/serialize.hpp"

#include "../errors.hpp"
#include "../log.hpp"
#include "../regions.hpp"

namespace epidb {
  namespace command {

    class ScoreMatrixCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::OPERATIONS, "Gets the regions for the given query in the requested BED format.");
      }

      static Parameters parameters_()
      {
        Parameter p[] = {
          Parameter("experiments_format", serialize::MAP, "id of the query"),
          Parameter("aggregation_function", serialize::STRING, "aggregation function"),
          Parameter("regions_query_id", serialize::STRING, "regions query id"),
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 4);
        return params;
      }

      static Parameters results_()
      {
        Parameter p[] = {
          Parameter("regions", serialize::STRING, "BED formated regions")
        };
        Parameters results(&p[0], &p[0] + 1);
        return results;
      }

    public:
      ScoreMatrixCommand() : Command("score_matrix", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        const std::string aggregation_function = parameters[1]->as_string();
        const std::string regions_query_id = parameters[2]->as_string();
        const std::string user_key = parameters[3]->as_string();

        bool ok = false;
        std::string msg;
        if (!Command::checks(user_key, msg)) {
          result.add_error(msg);
          return false;
        }

        if (!dba::check_query(user_key, regions_query_id, ok, msg)) {
          result.add_error(msg);
          return false;
        }
        if (!ok) {
          result.add_error("Invalid query id.");
          return false;
        }

        // TODO Check same experiments biosource
        std::map<std::string, serialize::ParameterPtr> map_;
        if (!parameters[0]->children(map_)) {
          result.add_error("unable to read metadata");
          return false;
        }
        std::vector<std::pair<std::string, std::string>> experiments_formats;
        std::map<std::string, serialize::ParameterPtr>::iterator mit;
        for (mit = map_.begin(); mit != map_.end(); ++mit) {
          std::pair<std::string, std::string> p(mit->first, mit->second->as_string());
          experiments_formats.push_back(p);
        }


        ChromosomeRegionsList range_regions;
        if (!dba::query::retrieve_query(user_key, regions_query_id, range_regions, msg)) {
          result.add_error(msg);
          return false;
        }

        /// Experiments, Chromosomes, Values
        std::string norm_genome;

        // Change names to norm_names and columns
        std::vector<std::pair<std::string, std::string>> norm_experiments_formats;
        for (auto &experiment_input : experiments_formats) {
          mongo::BSONObj experiment;
          const std::string &experiment_name = experiment_input.first;
          if (!dba::experiments::by_name(experiment_name, experiment, msg)) {
            result.add_error(msg);
            return false;
          }
          std::string norm_name = experiment["norm_name"].String();

          std::string short_column;
          if (!dba::KeyMapper::to_short(experiment_input.second, short_column, msg)) {
            result.add_error(msg);
          }

          std::pair<std::string, std::string> p(norm_name, short_column);
          norm_experiments_formats.push_back(p);

          // TODO: check if are all from the same genome
          norm_genome = experiment["norm_genome"].String();
        }

        std::map<std::string, std::vector<std::pair<Region, std::vector<algorithms::Accumulator>>>> chromosome_accs;
        for (auto &chromosome : range_regions) {
          // Get the accumulators from each region
          std::vector<std::pair<Region, std::vector<algorithms::Accumulator>>> regions_accs;
          for (auto &region : *chromosome.second) {
            // Get the accumulator from each experiment
            std::vector<algorithms::Accumulator> experiments_accs;
            for (auto &experiment_format : norm_experiments_formats) {
              mongo::BSONObj regions_query;
              if (!dba::query::build_experiment_query(region.start(), region.end(), experiment_format.first, user_key, regions_query, msg)) {
                result.add_error(msg);
                return false;
              }

              Regions regions;
              if (!dba::retrieve::get_regions(norm_genome, chromosome.first, regions_query, regions, msg)) {
                return false;
              }
              algorithms::Accumulator acc;
              for (auto &experiment_region : *regions) {
                acc.push(experiment_region.value(experiment_format.second));
              }
              experiments_accs.push_back(acc);
            }
            std::pair<Region, std::vector<algorithms::Accumulator> > region_accs = std::pair<Region, std::vector<algorithms::Accumulator> >(region, experiments_accs);
            regions_accs.push_back(region_accs);
          }
          chromosome_accs[chromosome.first] = regions_accs;
        }


        std::stringstream ss;
        ss << "CHROMOSOME\t";
        ss << "START\t";
        ss << "END\t";
        bool first = true;
	for (auto &experiments_format : experiments_formats) {
          if (!first) {
            ss << "\t";
          }
          ss << experiments_format.first;
          first = false;
        }
        ss << std::endl;
        for (std::map<std::string, std::vector<std::pair<Region, std::vector<algorithms::Accumulator>>>>::iterator chromosome_accs_it = chromosome_accs.begin(); chromosome_accs_it != chromosome_accs.end(); chromosome_accs_it++ ) {
          const std::string &chromosome = chromosome_accs_it->first;
          std::vector<std::pair<Region, std::vector<algorithms::Accumulator>>> &region_accs = chromosome_accs_it->second;
          for (std::vector<std::pair<Region, std::vector<algorithms::Accumulator>>>::iterator regions_accs_it = region_accs.begin(); regions_accs_it != region_accs.end(); regions_accs_it++) {

            Region region = regions_accs_it->first;
            std::vector<algorithms::Accumulator> &experiments_accs = regions_accs_it->second;

            ss << chromosome << "\t";
            ss << region.start() << "\t";
            ss << region.end() << "\t";

            for (std::vector<algorithms::Accumulator>::iterator accs_it = experiments_accs.begin();
                 accs_it != experiments_accs.end(); accs_it++) {

              std::string value = accs_it->string("|");
              if (value.empty()) {
                ss << "NA" << "\t";
              } else {
                ss << accs_it->string("|") << "\t";
              }
            }
            ss << std::endl;
          }
        }

        result.add_string(ss.str());

        return true;
      }

    } scoreMatrixCommand;
  }
}
