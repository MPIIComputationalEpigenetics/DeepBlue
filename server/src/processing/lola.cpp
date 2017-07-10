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

#include <algorithm> // for min and max

using namespace std;


template<int M, template<typename> class F = std::less>
struct TupleCompare {
  template<typename T>
  bool operator()(T const &t1, T const &t2)
  {
    return F<typename tuple_element<M, T>::type>()(std::get<M>(t1), std::get<M>(t2));
  }
};

namespace epidb {
  namespace processing {

    // [0] dataseset, [1] size, [2] database_name, [3] negative_natural_log, [4] log_odds_score, [5] a, [6] b, [7] c, [8] d,)
    using ProcessOverlapResult = std::tuple<std::string, int, std::string, double, double, int, int, int, int>;

    ProcessOverlapResult process_overlap(const std::string& genome, const std::vector<std::string>& chromosomes,
                                         const ChromosomeRegionsList &query_regions, const long total_query_regions,
                                         const std::string dataset, const std::string database_name,
                                         const ChromosomeRegionsList &universe_regions, const long total_universe_regions,
                                         processing::StatusPtr status, threading::SemaphorePtr sem,
                                         const std::string& user_key, std::string& msg)
    {
      ChromosomeRegionsList database_regions;
      if (utils::is_id(dataset, "q")) {
        if (!dba::query::retrieve_query(user_key, dataset, status, database_regions, msg, /* reduced_mode */ true)) {
          sem->up();
          return std::make_tuple(msg, -1.0, "", -1.0, -1.0, -1.0, -1.0, -1.0, -1.0);
        }
      } else {
        mongo::BSONObj regions_query;
        if (!dba::query::build_experiment_query(-1, -1, dataset, regions_query, msg)) {
          sem->up();
          return std::make_tuple(msg, -1.0, "", -1.0, -1.0, -1.0, -1.0, -1.0, -1.0);
        }
        if (!dba::retrieve::get_regions(genome, chromosomes, regions_query, false, status, database_regions, msg, /* reduced_mode */ true)) {
          sem->up();
          return std::make_tuple(msg, -1.0, "", -1.0, -1.0, -1.0, -1.0, -1.0, -1.0);
        }
      }

      size_t count_database_regions = count_regions(database_regions);
      size_t query_overlap_total;
      if (!algorithms::intersect_count(query_regions, database_regions, query_overlap_total)) {
        sem->up();
        return std::make_tuple(msg, -1.0, "", -1.0, -1.0, -1.0, -1.0, -1.0, -1.0);
      }

      // a = #query_overlap_with_universe overlapping with at least one region in dataset_regions
      double a = query_overlap_total;

      /*
        b - the # of items *in the universe* that overlap each dbSet less the support;
        This is the number of items in the universe that are in the dbSet ONLY (not in userSet).
        b = #query_overlap_with_universe NOT overlapping with at least one region in dataset_regions
      */
      size_t testSetsOverlapUniverse;
      if (!algorithms::intersect_count(universe_regions, database_regions, testSetsOverlapUniverse)) {

      }
      double b = testSetsOverlapUniverse - a;

      if (b < 0) {
        sem->up();
        msg = "Negative b entry in table. This means either: 1) Your user sets contain items outside your universe; or 2) your universe has a region that overlaps multiple user set regions, interfering with the universe set overlap calculation.";
        return std::make_tuple(msg, -1.0, "", -1.0, -1.0, -1.0, -1.0, -1.0, -1.0);
      }

      /*
       c - [non-hits in user query set]. For this I take the size of the user set - support
       this is the rest of the user set that did not have any overlaps to the test set.
      */
      // c = #universe_overlap_with_dataset that are NOT contained in query
      double c = total_query_regions  - a;

      //# d - [size of universe - b -c -a]
      //#universe_regions - a - b - c = #universe_regions that do NOT overlap with query_set and that do NOT overlap with dataset_regions
      double d = total_universe_regions - a - b - c;

      std::cerr << "[starting] " << dataset;
      std::cerr <<"(" << a << ", " << b << ", " << c << ", " << d << ") negative natural log: ";

      double p_value = math::fisher_test(a, b, c, d);
      double negative_natural_log = abs(log10(p_value));

      std::cerr << "p_value: " << p_value << " " << "negative_natural_log: " << negative_natural_log;

      double log_odds_score = (a/b)/(c/d);
      std::cerr << " log odds: " << log_odds_score << std::endl;

      status->subtract_regions(count_database_regions);
      for (const auto& chr : database_regions) {
        for (const auto& rs :chr.second) {
          status->subtract_size(rs->size());
        }
      }

      sem->up();
      return std::make_tuple(dataset, count_database_regions, database_name, negative_natural_log, log_odds_score, a, b, c, d);
    }


    bool lola(const std::string& query_id, const std::string& universe_query_id,
              const mongo::BSONObj& databases,
              const std::string& genome,
              const std::string& user_key,
              processing::StatusPtr status, mongo::BSONObj& result, std::string& msg)
    {
      INIT_PROCESSING(PROCESS_LOLA, status)

      ChromosomeRegionsList query_regions;
      if (!dba::query::retrieve_query(user_key, query_id, status, query_regions, msg, /* reduced_mode */ true)) {
        return false;
      }
      size_t total_query_regions = count_regions(query_regions);
      std::cerr << total_query_regions << std::endl;

      std::cerr << "LOADING UNIVERSE" << std::endl;
      long times = clock();
      ChromosomeRegionsList universe_regions;
      if (!dba::query::retrieve_query(user_key, universe_query_id, status, universe_regions, msg, /* reduced_mode */ true)) {
        return false;
      }
      size_t total_universe_regions = count_regions(universe_regions);
      std::cerr << "before " << total_universe_regions << std::endl;

      ChromosomeRegionsList disjoin_set = algorithms::disjoin(std::move(universe_regions));
      size_t disjoin_set_count = count_regions(disjoin_set);
      std::cerr << "disjoin_set: " << disjoin_set_count << std::endl;
      universe_regions = std::move(disjoin_set);
      long diffticks = clock() - times;
      std::cerr << "Load universe: " << ((diffticks) / (CLOCKS_PER_SEC / 1000)) << std::endl;

      std::set<std::string> chromosomes_s;
      const std::string& norm_genome = utils::normalize_name(genome);
      if (!dba::genomes::get_chromosomes(norm_genome, chromosomes_s, msg)) {
        return false;
      }
      std::vector<std::string> chromosomes(chromosomes_s.begin(), chromosomes_s.end());

      auto databases_it = databases.begin();

      std::vector<std::tuple<std::string, size_t>> datasets_support;
      std::vector<std::tuple<std::string, double>> datasets_log_score;
      std::vector<std::tuple<std::string, double>> datasets_odds_score;

      std::vector<std::future<ProcessOverlapResult > > threads;

      threading::SemaphorePtr sem = threading::build_semaphore(32);

      while ( databases_it.more() ) {
        const mongo::BSONElement &database = databases_it.next();
        const std::string& database_name = std::string(database.fieldName());

        const auto& datasets = database.Obj();

        auto datasets_it = datasets.begin();
        while (datasets_it.more()) {
          long times = clock();
          const auto& dataset = datasets_it.next().str();

          sem->down();
          auto t = std::async(std::launch::async, &process_overlap,
                              std::ref(genome), std::ref(chromosomes),
                              std::ref(query_regions), total_query_regions,
                              dataset, database_name,
                              std::ref(universe_regions), total_universe_regions,
                              status, sem,
                              std::ref(user_key), std::ref(msg));

          threads.emplace_back(std::move(t));
        }
      }


      std::vector<ProcessOverlapResult> results;

      for (size_t i = 0; i < threads.size(); ++i) {
        threads[i].wait();
        auto result = threads[i].get();
        if (std::get<2>(result).empty()) {
          msg = std::get<0>(result);
          return false;
        }

        std::string dataset = std::get<0>(result);

        // --------------------- [0] dataseset, [1] size, [2] database_name, [3] negative_natural_log, [4] log_odds_score, [5] a, [6] b, [7] c, [8] d,)
        datasets_support.push_back(std::make_tuple(dataset, std::get<5>(result)));
        datasets_log_score.push_back(std::make_tuple(dataset, std::get<3>(result)));
        datasets_odds_score.push_back(std::make_tuple(dataset, std::get<4>(result)));

        results.emplace_back(std::move(result));
      }

      std::unordered_map<std::string, int> datasets_support_rank;
      std::sort(begin(datasets_support), end(datasets_support), TupleCompare<1>());
      for(size_t i = 0; i < datasets_support.size(); i++) {
        datasets_support_rank[get<0>(datasets_support[i])] = i;
      }
      //

      std::unordered_map<std::string, int> datasets_log_rank;
      std::sort(begin(datasets_log_score), end(datasets_log_score), TupleCompare<1>());
      for(size_t i = 0; i < datasets_log_score.size(); i++) {
        datasets_log_rank[get<0>(datasets_log_score[i])] = i;
      }
      //

      std::unordered_map<std::string, int> datasets_odd_rank;
      std::sort(begin(datasets_odds_score), end(datasets_odds_score), TupleCompare<1>());
      for(size_t i = 0; i < datasets_odds_score.size(); i++) {
        datasets_odd_rank[get<0>(datasets_odds_score[i])] = i;
      }

      struct ExperimentResult {
        std::string dataset;

        int experiment_size;

        std::string database_name;

        double negative_natural_log;
        double log_odds_ratio;

        int a;
        int b;
        int c;
        int d;

        int support_rank;
        int log_rank;
        int odd_rank;

        int max_rank;
        double mean_rank;

        mongo::BSONObj toBSON()
        {
          return BSON(
                   "dataset" << dataset <<
                   "experiment_size" << experiment_size <<
                   "database_name" << database_name <<
                   "p_value_log" << negative_natural_log <<
                   "log_odds_ratio" << log_odds_ratio <<
                   "support" << a <<
                   "b" << b <<
                   "c" << c <<
                   "d" << d <<
                   "support_rank" << support_rank <<
                   "log_rank" << log_rank <<
                   "odd_rank" << odd_rank <<
                   "max_rank" << max_rank <<
                   "mean_rank" << mean_rank
                 );
        }
      };

      struct ERCompare {
        bool operator()(std::shared_ptr<ExperimentResult> const &er, std::shared_ptr<ExperimentResult> const &er2)
        {
          return er->mean_rank < er2->mean_rank;
        }
      };

      std::vector<std::shared_ptr<ExperimentResult>> experiment_results;

      for (const auto& result : results) {
        auto er = std::make_shared<ExperimentResult>();
        er->dataset = get<0>(result);
        er->experiment_size = get<1>(result);
        er->database_name = get<2>(result);
        er->negative_natural_log = get<3>(result);
        er->log_odds_ratio = get<4>(result);
        er->a = get<5>(result);
        er->b = get<6>(result);
        er->c = get<7>(result);
        er->d = get<8>(result);

        er->support_rank = datasets_support_rank[er->dataset];
        er->log_rank = datasets_log_rank[er->dataset];
        er->odd_rank = datasets_odd_rank[er->dataset];
        er->max_rank = std::max(er->support_rank, std::max(er->log_rank, er->odd_rank));
        er->mean_rank = (er->support_rank + er->log_rank + er->odd_rank) / 3.0;

        experiment_results.emplace_back(std::move(er));
      }

      std::sort(begin(experiment_results), end(experiment_results), ERCompare());
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