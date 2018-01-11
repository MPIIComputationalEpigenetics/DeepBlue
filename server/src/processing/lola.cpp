//
//  lola.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 23.06.2017.
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

#include <algorithm> // for min and max
#include <future>
#include <iterator>
#include <string>
#include <thread>

#include "../algorithms/algorithms.hpp"

#include "../dba/experiments.hpp"
#include "../dba/genomes.hpp"
#include "../dba/genes.hpp"
#include "../dba/gene_ontology.hpp"
#include "../dba/queries.hpp"
#include "../dba/retrieve.hpp"

#include "../extras/math.hpp"
#include "../extras/utils.hpp"

#include "../threading/semaphore.hpp"

#include "enrichment_result.hpp"


namespace epidb {
  namespace processing {

    ProcessOverlapResult process_overlap(const datatypes::User& user,
                                         const std::string& genome, const std::vector<std::string>& chromosomes,
                                         const ChromosomeRegionsList &redefined_universe_overlap_query, const long count_redefined_universe_overlap_query,
                                         const std::string dataset_name, const std::string description,
                                         const std::string database_name,
                                         const ChromosomeRegionsList &universe_regions, const long total_universe_regions,
                                         processing::StatusPtr status, threading::SemaphorePtr sem,
                                         std::string& msg)
    {
      ChromosomeRegionsList database_regions;

      std::string biosource;
      std::string epigenetic_mark;

      if (utils::is_id(dataset_name, "q")) {
        if (!dba::query::retrieve_query(user, dataset_name, status, database_regions, msg, /* reduced_mode */ true)) {
          sem->up();
          return std::make_tuple(dataset_name, biosource, epigenetic_mark, description, -1, database_name, -1, -1, -1, -1, -1, -1, true, msg);
        }

        std::vector<std::string> exp_biosources;
        const std::string bs_field_name = "sample_info.biosource_name";
        if (!dba::query::get_main_experiment_data(user, dataset_name, bs_field_name, status, exp_biosources, msg)) {
          return std::make_tuple(dataset_name, biosource, epigenetic_mark, description, -1, database_name, -1, -1, -1, -1, -1, -1, true, msg);
        }

        std::vector<std::string> exp_epigenetic_marks;
        const std::string em_field_name = "epigenetic_mark";
        if (!dba::query::get_main_experiment_data(user, dataset_name, em_field_name, status, exp_epigenetic_marks, msg)) {
          return std::make_tuple(dataset_name, biosource, epigenetic_mark, description, -1, database_name, -1, -1, -1, -1, -1, -1, true, msg);
        }

        biosource = utils::vector_to_string(exp_biosources);
        epigenetic_mark = utils::vector_to_string(exp_epigenetic_marks);
      } else {
        mongo::BSONObj regions_query;

        if (!dba::query::build_experiment_query(-1, -1, utils::normalize_name(dataset_name), regions_query, msg)) {
          sem->up();
          return std::make_tuple(dataset_name, biosource, epigenetic_mark, description, -1, database_name, -1, -1, -1, -1, -1, -1, true, msg);
        }

        if (!dba::retrieve::get_regions(genome, chromosomes, regions_query, false, status, database_regions, msg, /* reduced_mode */ true)) {
          sem->up();
          return std::make_tuple(dataset_name, biosource, epigenetic_mark, description, -1, database_name, -1, -1, -1, -1, -1, -1, true, msg);
        }

        mongo::BSONObj experiment_obj;
        if (!dba::experiments::by_name(dataset_name, experiment_obj, msg)) {
          sem->up();
          return std::make_tuple(dataset_name, biosource, epigenetic_mark, description, -1, database_name, -1, -1, -1, -1, -1, -1, true, msg);
        }

        if (!experiment_obj.isEmpty()) {
          biosource = experiment_obj["sample_info"]["biosource_name"].str();
          epigenetic_mark = experiment_obj["epigenetic_mark"].str();
        }
      }


      size_t count_database_regions = count_regions(database_regions);


      // This will become "support" -- the number of regions in the
      // userSet (which I implicitly assume is ALSO the number of regions
      // in the universe) that overlap anything in each database set.
      // Turn results into an overlap matrix. It is
      // dbSets (rows) by userSets (columns), counting overlap.
      size_t query_overlap_total;
      if (!algorithms::intersect_count(redefined_universe_overlap_query, database_regions, query_overlap_total)) {
        sem->up();
        return std::make_tuple(dataset_name, biosource, epigenetic_mark, description, count_database_regions, database_name, -1, -1, -1, -1, -1, -1, true, msg);
      }
      double a = query_overlap_total;


      // b = the # of items *in the universe* that overlap each dbSet,
      // less the support; This is the number of items in the universe
      // that are in the dbSet ONLY (not in userSet)
      size_t universe_overlap_with_database_total;
      if (!algorithms::intersect_count(universe_regions, database_regions, universe_overlap_with_database_total)) {
        sem->up();
        return std::make_tuple(dataset_name, biosource, epigenetic_mark, description, count_database_regions, database_name, -1, -1, -1, -1, -1, -1, true, msg);
      }
      double b = universe_overlap_with_database_total - a;


      // c = the size of userSet, less the support; This is the opposite:
      // Items in the userSet ONLY (not in the dbSet)
      double c = count_redefined_universe_overlap_query - a;


      //#universe_regions - a - b - c = #universe_regions that do NOT overlap with query_set and that do NOT overlap with dataset_regions
      double d = total_universe_regions - a - b - c;


      if (b < 0) {
        sem->up();
        msg = "Negative b entry in table. This means either: 1) Your user sets contain items outside your universe; or 2) your universe has a region that overlaps multiple user set regions, interfering with the universe set overlap calculation. Dataset: " + dataset_name;
        return std::make_tuple(dataset_name, biosource, epigenetic_mark, description, count_database_regions, database_name, -1, -1, a, b, c, d, true, msg);
      }

      if (d < 0) {
        sem->up();
        msg = "Negative d entry in table. This means either: 1) Your user sets contain items outside your universe; or 2) your universe has a region that overlaps multiple user set regions, interfering with the universe set overlap calculation. Dataset: " + dataset_name ;
        return std::make_tuple(dataset_name, biosource, epigenetic_mark, description, count_database_regions, database_name, -1, -1, a, b, c, d, true, msg);
      }

      double p_value = math::fisher_test(a, b, c, d);
      double negative_natural_log = abs(log10(p_value));

      double a_b = a/b;
      double c_d = c/d;
      double odds_score;
      // Handle when 'b' and 'd' are 0.
      if (a_b == c_d) {
        odds_score = 1;
      } else {
        odds_score = a_b/c_d;
      }

      status->subtract_regions(count_database_regions);
      for (const auto& chr : database_regions) {
        for (const auto& rs :chr.second) {
          status->subtract_size(rs->size());
        }
      }

      sem->up();
      return std::make_tuple(dataset_name, biosource, epigenetic_mark, description, count_database_regions, database_name, negative_natural_log, odds_score, a, b, c, d, false, "");
    }


    bool lola(const datatypes::User& user,
              const std::string& query_id, const std::string& universe_query_id,
              const mongo::BSONObj& databases,
              const std::string& genome,
              processing::StatusPtr status, mongo::BSONObj& result, std::string& msg)
    {
      IS_PROCESSING_CANCELLED(status);
      processing::RunningOp runningOp =  status->start_operation(PROCESS_ENRICH_REGIONS_OVERLAP);


      ChromosomeRegionsList query_regions;
      if (!dba::query::retrieve_query(user, query_id, status, query_regions, msg, /* reduced_mode */ true)) {
        return false;
      }
      size_t total_query_regions = count_regions(query_regions);
      long times = clock();
      ChromosomeRegionsList universe_regions;
      if (!dba::query::retrieve_query(user, universe_query_id, status, universe_regions, msg, /* reduced_mode */ true)) {
        return false;
      }
      size_t total_universe_regions = count_regions(universe_regions);

      ChromosomeRegionsList disjoin_set = algorithms::disjoin(std::move(universe_regions));
      size_t disjoin_set_count = count_regions(disjoin_set);

      universe_regions = std::move(disjoin_set);
      ChromosomeRegionsList redefined_universe_overlap_query;
      if (!algorithms::intersect(universe_regions, query_regions, status, redefined_universe_overlap_query, msg)) {
        return false;
      }
      size_t count_redefined_universe_overlap_query = count_regions(redefined_universe_overlap_query);


      std::set<std::string> chromosomes_s;
      const std::string& norm_genome = utils::normalize_name(genome);
      if (!dba::genomes::get_chromosomes(norm_genome, chromosomes_s, msg)) {
        return false;
      }
      std::vector<std::string> chromosomes(chromosomes_s.begin(), chromosomes_s.end());

      std::vector<std::tuple<std::string, size_t>> datasets_support;
      std::vector<std::tuple<std::string, double>> datasets_log_score;
      std::vector<std::tuple<std::string, double>> datasets_odds_score;

      std::vector<std::future<ProcessOverlapResult > > threads;

      threading::SemaphorePtr sem = threading::build_semaphore(64);

      auto databases_it = databases.begin();
      while ( databases_it.more() ) {
        const mongo::BSONElement &database = databases_it.next();
        const std::string& database_name = std::string(database.fieldName());

        const auto& datasets = database.Obj();

        auto datasets_it = datasets.begin();
        while (datasets_it.more()) {
          long times = clock();
          const auto& dataset = datasets_it.next().Obj();
          const auto& dataset_name = dataset["name"].String();
          const auto& description = dataset["description"].String();

          sem->down();
          auto t = std::async(std::launch::async, &process_overlap,
                              std::ref(user),
                              std::ref(genome), std::ref(chromosomes),
                              std::ref(redefined_universe_overlap_query), count_redefined_universe_overlap_query,
                              dataset_name, description, database_name,
                              std::ref(universe_regions), total_universe_regions,
                              status, sem,
                              std::ref(msg));

          threads.emplace_back(std::move(t));
        }
      }

      std::vector<ProcessOverlapResult> results;

      for (size_t i = 0; i < threads.size(); ++i) {
        threads[i].wait();
        auto result = threads[i].get();

        std::string dataset = std::get<0>(result);

        datasets_log_score.push_back(std::make_tuple(dataset, std::get<6>(result)));
        datasets_odds_score.push_back(std::make_tuple(dataset, std::get<7>(result)));
        datasets_support.push_back(std::make_tuple(dataset, std::get<8>(result)));

        results.emplace_back(std::move(result));
      }

      std::vector<std::shared_ptr<ExperimentResult>> experiment_results =
            sort_results(results, datasets_support, datasets_log_score, datasets_odds_score);

      mongo::BSONObjBuilder bob;
      bob.append("count_query_regions", (int) total_query_regions);
      bob.append("count_universe_regions", (int) total_universe_regions);

      mongo::BSONArrayBuilder ab;
      for (const auto& er: experiment_results) {
        ab.append(er->toBSON());
      }
      bob.append("results", ab.obj());

      result = bob.obj();

      return true;
    }
  }
}