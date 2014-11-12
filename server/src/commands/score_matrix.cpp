//
//  score_matrix.cpp
//  epidb
//
//  Created by Felipe Albrecht on 08.11.14.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

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
          Parameter("experiments_id", serialize::STRING, "id of the query", true),
          Parameter("column_name", serialize::STRING, "column name"),
          Parameter("aggregation_function", serialize::STRING, "aggregation function"),
          Parameter("regions_query_id", serialize::STRING, "regions query id"),
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 5);
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
        const std::string column_name = parameters[1]->as_string();
        const std::string aggregation_function = parameters[2]->as_string();
        const std::string regions_query_id = parameters[3]->as_string();
        const std::string user_key = parameters[4]->as_string();

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


        std::string short_column;
        if (!dba::KeyMapper::to_short(column_name, short_column, msg)) {
          result.add_error(msg);
        }

        // TODO Check same experiments biosource

        std::vector<serialize::ParameterPtr> experiment_names;
        parameters[0]->children(experiment_names);
        std::vector<std::string> experiments = dba::helpers::build_vector(experiment_names);

        ChromosomeRegionsList regions;

        ChromosomeRegionsList range_regions;
        if (!dba::query::retrieve_query(user_key, regions_query_id, range_regions, msg)) {
          result.add_error(msg);
          return false;
        }

        for (auto &experiment_name : experiments) {
          mongo::BSONObj experiment;
          if (!dba::experiments::by_name(experiment_name, experiment, msg)) {
            result.add_error(msg);
            return false;
          }

          const std::string norm_genome = experiment["norm_genome"].String();

          for (auto &chromosome : range_regions) {
            for (auto &region : *chromosome.second) {
              mongo::BSONObj regions_query;
              std::cerr << region.start() <<  " " << region.end() << std::endl;
              if (!dba::query::build_experiment_query(region.start(), region.end(), experiments[0], user_key, regions_query, msg)) {
                result.add_error(msg);
                return false;
              }
              Regions regions;
              if (!dba::retrieve::get_regions(norm_genome, chromosome.first, regions_query, regions, msg)) {
                return false;
              }
              //algorithms::Accumulator acc;
              //for (auto &experiment_region : *regions) {
              //  acc.push(experiment_region.value(short_column));
              //}
              //std::cerr << acc.string("|") << std::endl;
            }

          }
        }

        return true;
      }

    } scoreMatrixCommand;
  }
}
