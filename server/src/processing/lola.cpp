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

#include <iterator>
#include <string>

#include "../algorithms/algorithms.hpp"

#include "../dba/experiments.hpp"
#include "../dba/genomes.hpp"
#include "../dba/genes.hpp"
#include "../dba/gene_ontology.hpp"
#include "../dba/queries.hpp"
#include "../dba/retrieve.hpp"

#include "../extras/math.hpp"
#include "../extras/utils.hpp"


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
    bool lola(const std::string& query_id, const std::string& universe_query_id,
              const mongo::BSONObj& databases,
              const std::string& genome,
              const std::string& user_key,
              processing::StatusPtr status, mongo::BSONObj& result, std::string& msg)
    {
      INIT_PROCESSING(PROCESS_LOLA, status)

      std::cerr << query_id << std::endl;

      std::cerr << universe_query_id << std::endl;

      ChromosomeRegionsList queryChromosomeRegionsList;
      if (!dba::query::retrieve_query(user_key, query_id, status, queryChromosomeRegionsList, msg, /* reduced_mode */ true)) {
        return false;
      }
      size_t total_query_regions = count_regions(queryChromosomeRegionsList);
      std::cerr << total_query_regions << std::endl;

      std::cerr << "LOADING UNIVERSE" << std::endl;
      long times = clock();
      ChromosomeRegionsList universeChromosomeRegionsList;
      if (!dba::query::retrieve_query(user_key, universe_query_id, status, universeChromosomeRegionsList, msg, /* reduced_mode */ true)) {
        return false;
      }
      size_t total_universe_regions = count_regions(universeChromosomeRegionsList);
      std::cerr << total_universe_regions << std::endl;
      long diffticks = clock() - times;
      std::cerr << "Load universe: " << ((diffticks) / (CLOCKS_PER_SEC / 1000)) << std::endl;



      ChromosomeRegionsList query_overlap_with_universe;
      if (!algorithms::intersect(queryChromosomeRegionsList, universeChromosomeRegionsList, query_overlap_with_universe)) {
        return false;
      }


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


      // --------------------- dataset_id, dataseset_name, database_name, negative_natural_log, 0, a, b, c, d,)
      std::vector<std::tuple<std::string, std::string, std::string, double, double, double, double, double, double>> results;


      while ( databases_it.more() ) {
        const mongo::BSONElement &database = databases_it.next();
        const std::string& database_name = std::string(database.fieldName());

        const auto& datasets = database.Obj();

        auto datasets_it = datasets.begin();
        while (datasets_it.more()) {
          long times = clock();
          const auto& experiment_name = datasets_it.next().str();
          std::cerr << experiment_name << " - ";

          mongo::BSONObj regions_query;
          if (!dba::query::build_experiment_query(-1, -1, experiment_name, regions_query, msg)) {
            return false;
          }

          mongo::BSONObj experiment_obj;
          if (!dba::experiments::by_name(experiment_name, experiment_obj, msg)) {
            return false;
          }

          const std::string dataset_id = experiment_obj["_id"].str();

          ChromosomeRegionsList database_regions;
          if (!dba::retrieve::get_regions(genome, chromosomes, regions_query, false, status, database_regions, msg, /* reduced_mode */ true)) {
            return false;
          }

          size_t query_overlap_total;
          if (!algorithms::intersect_count(database_regions, query_overlap_with_universe, query_overlap_total)) {
            return false;
          }

          /*
           a - [support]. The number in the user query set that overlaps with at least 1 region.
          */
          double a = query_overlap_total;

          /*
            b - the # of items *in the universe* that overlap each dbSet less the support;
            This is the number of items in the universe
          that are in the dbSet ONLY (not in userSet).
          */
          size_t universe_overlap_total;
          if (!algorithms::intersect_count(universeChromosomeRegionsList, database_regions, universe_overlap_total)) {
            return false;
          }

          double b = universe_overlap_total - a;

          /*
           c - [non-hits in user query set]. For this I take the size of the user set - support
           this is the rest of the user set that did not have any overlaps to the test set.
          */
          double c = total_query_regions - a;

          //# d - [size of universe - b -c -a]
          double d = total_universe_regions - b - c - a;

          std::cerr <<"(" << a << ", " << b << ", " << c << ", " << d << ") log odds: ";

          double p_value = math::fisher_test(a, b, c, d);
          double negative_natural_log = abs(log(p_value));

          std::cerr << negative_natural_log << " ";

          long diffticks = clock() - times;
          std::cerr << " time: " << ((diffticks) / (CLOCKS_PER_SEC / 1000)) << std::endl;

          double log_odds_score = log((a/b)/(c/d));

          datasets_support.push_back(std::make_tuple(dataset_id, a));
          datasets_log_score.push_back(std::make_tuple(dataset_id, negative_natural_log));
          datasets_odds_score.push_back(std::make_tuple(dataset_id, log_odds_score));

          /*
           @return Data.table with enrichment results.
           Rows correspond to individual pairwise fisher's tests comparing a single userSet with a single databaseSet.

           The columns in this data.table are:
            - userSet
            - and dbSet: index into their respective input region sets.
            - pvalueLog: -log10(pvalue) from the fisher's exact result;
            - oddsRatio: result from the fisher's exact test;
            - support: number of regions in userSet overlapping databaseSet;
            - rnkPV: rank in this table of p-value
            - rnkLO: rank in this table of oddsRatio
            - rnkSup: rank in this table of Support respectively.
            - The --value is the negative natural log of the p-value returned from a one-sided fisher's exact test.
            - maxRnk: max of the 3 previous ranks, providing a combined ranking system
            - meanRnk: mean of the 3 previous ranks, providing a combined ranking system.
            - b: other value completing the 2x2 contingency table (with support).
            - c: other value completing the 2x2 contingency table (with support).
            - d: other value completing the 2x2 contingency table (with support).
          */


          results.push_back(std::make_tuple(dataset_id, experiment_name, database_name, negative_natural_log, log_odds_score, a, b, c, d));
        }
      }

      std::unordered_map<std::string, size_t> datasets_support_rank;
      std::sort(begin(datasets_support), end(datasets_support), TupleCompare<1>());
      for(size_t i = 0; i < datasets_support.size(); i++) {
        datasets_support_rank[get<0>(datasets_support[i])] = i;
      }
      //

      std::unordered_map<std::string, size_t> datasets_log_rank;
      std::sort(begin(datasets_log_score), end(datasets_log_score), TupleCompare<1>());
      for(size_t i = 0; i < datasets_log_score.size(); i++) {
        datasets_log_rank[get<0>(datasets_log_score[i])] = i;
      }
      //

      std::unordered_map<std::string, size_t> datasets_odd_rank;
      std::sort(begin(datasets_odds_score), end(datasets_odds_score), TupleCompare<1>());
      for(size_t i = 0; i < datasets_odds_score.size(); i++) {
        datasets_odd_rank[get<0>(datasets_odds_score[i])] = i;
      }

      struct ExperimentResult {
        std::string dataset_id;
        std::string experiment_name;
        std::string database_name;

        double negative_natural_log;
        double log_odds_ratio;

        double a;
        double b;
        double c;
        double d;

        int support_rank;
        int log_rank;
        int odd_rank;

        int max_rank;
        double mean_rank;

        mongo::BSONObj toBSON()
        {
          return BSON(
                   "experiment_id" << dataset_id <<
                   "experiment_name" << experiment_name <<
                   "database_name" << database_name <<
                   "p_value_log" << negative_natural_log <<
                   "log_odds_ratio" << log_odds_ratio <<
                   "support" << (int) a <<
                   "b" << (int) b <<
                   "c" << (int) c <<
                   "d" << (int) d <<
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
        er->dataset_id = get<0>(result);
        er->experiment_name = get<1>(result);
        er->database_name = get<2>(result);
        er->negative_natural_log = get<3>(result);
        er->log_odds_ratio = get<4>(result);
        er->a = get<5>(result);
        er->b = get<6>(result);
        er->c = get<7>(result);
        er->d = get<8>(result);

        er->support_rank = datasets_support_rank[er->dataset_id];
        er->log_rank = datasets_log_rank[er->dataset_id];
        er->odd_rank = datasets_odd_rank[er->dataset_id];
        er->max_rank = std::max(er->support_rank, std::max(er->log_rank, er->odd_rank));
        er->mean_rank = (er->support_rank + er->log_rank + er->odd_rank) / 3.0;

        experiment_results.emplace_back(std::move(er));
      }

      std::sort(begin(experiment_results), end(experiment_results), ERCompare());


      mongo::BSONArrayBuilder ab;


      for (const auto& er: experiment_results) {
        ab.append(er->toBSON());
      }

      std::cerr << ab.obj().toString() << std::endl;

      return true;
    }
  }
}