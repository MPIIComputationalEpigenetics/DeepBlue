//
//  enrichment_result.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 25.10.17.
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
#include <string>
#include <unordered_map>
#include <vector>

#include <mongo/bson/bson.h>


using namespace std;

template<int M, template<typename> class F = std::less>
struct TupleCompare {
  template<typename T>
  bool operator()(T const &t1, T const &t2)
  {
    return F<typename tuple_element<M, T>::type>()(std::get<M>(t2), std::get<M>(t1));
  }
};

namespace epidb {
  namespace processing {

    // [0] dataseset, [1] biosource, [2] epigenetic_mark, [3] descroption, [4] size, [5] database_name, [6] negative_natural_log, [7] log_odds_score, [8] a, [9] b, [10] c, [11] d,)
    using ProcessOverlapResult = std::tuple<std::string, std::string, std::string, std::string, int, std::string, double, double, int, int, int, int>;

    struct ExperimentResult {
      std::string dataset;
      std::string biosource;
      std::string epigenetic_mark;
      std::string description;

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
                 "biosource" << biosource <<
                 "epigenetic_mark" << epigenetic_mark <<
                 "description" << description <<
                 "experiment_size" << experiment_size <<
                 "database_name" << database_name <<
                 "p_value_log" << negative_natural_log <<
                 "odds_ratio" << log_odds_ratio <<
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

    void sort_values(std::vector<std::tuple<std::string, size_t>>& datasets_support,
                     std::vector<std::tuple<std::string, double>>& datasets_log_score,
                     std::vector<std::tuple<std::string, double>>& datasets_odds_score,
                     std::unordered_map<std::string, int>& datasets_support_rank,
                     std::unordered_map<std::string, int>& datasets_log_rank,
                     std::unordered_map<std::string, int>& datasets_odd_rank)
    {
      if (datasets_support.empty()) {
        return;
      }

      size_t position = 0;
      size_t s_value = get<1>(datasets_support[0]);
      std::sort(begin(datasets_support), end(datasets_support), TupleCompare<1>());
      for(size_t i = 0; i < datasets_support.size(); i++) {
        if (get<1>(datasets_support[i]) != s_value) {
          position = i;
          s_value = get<1>(datasets_support[i]);
        }
        datasets_support_rank[get<0>(datasets_support[i])] = position + 1;
      }

//
      position = 0;
      double d_value = get<1>(datasets_log_score[0]);
      std::sort(begin(datasets_log_score), end(datasets_log_score), TupleCompare<1>());
      for(size_t i = 0; i < datasets_log_score.size(); i++) {
        if (get<1>(datasets_log_score[i]) != d_value) {
          position = i;
          d_value = get<1>(datasets_log_score[i]);
        }
        datasets_log_rank[get<0>(datasets_log_score[i])] = position + 1;
      }

//
      position = 0;
      d_value = get<1>(datasets_odds_score[0]);
      std::sort(begin(datasets_odds_score), end(datasets_odds_score), TupleCompare<1>());
      for(size_t i = 0; i < datasets_odds_score.size(); i++) {
        if (get<1>(datasets_odds_score[i]) != d_value) {
          position = i;
          d_value = get<1>(datasets_odds_score[i]);
        }
        datasets_odd_rank[get<0>(datasets_odds_score[i])] = position + 1;
      }
    }

    std::vector<std::shared_ptr<ExperimentResult>> sort_results(
          const std::vector<ProcessOverlapResult>& results,
          std::vector<std::tuple<std::string, size_t>>& datasets_support,
          std::vector<std::tuple<std::string, double>>& datasets_log_score,
          std::vector<std::tuple<std::string, double>>& datasets_odds_score)
    {

      std::unordered_map<std::string, int> datasets_support_rank;
      std::unordered_map<std::string, int> datasets_log_rank;
      std::unordered_map<std::string, int> datasets_odd_rank;

      sort_values(datasets_support, datasets_log_score, datasets_odds_score,
                  datasets_support_rank, datasets_log_rank, datasets_odd_rank);

      std::vector<std::shared_ptr<ExperimentResult>> experiment_results;
      for (const auto& result : results) {
        auto er = std::make_shared<ExperimentResult>();
        er->dataset = get<0>(result);
        er->biosource = get<1>(result);
        er->epigenetic_mark = get<2>(result);
        er->description = get<3>(result);
        er->experiment_size = get<4>(result);
        er->database_name = get<5>(result);
        er->negative_natural_log = get<6>(result);
        er->log_odds_ratio = get<7>(result);
        er->a = get<8>(result);
        er->b = get<9>(result);
        er->c = get<10>(result);
        er->d = get<11>(result);

        er->support_rank = datasets_support_rank[er->dataset];
        er->log_rank = datasets_log_rank[er->dataset];
        er->odd_rank = datasets_odd_rank[er->dataset];
        er->max_rank = std::max(er->support_rank, std::max(er->log_rank, er->odd_rank));
        er->mean_rank = (er->support_rank + er->log_rank + er->odd_rank) / 3.0;

        experiment_results.emplace_back(std::move(er));
      }

      std::sort(begin(experiment_results), end(experiment_results), ERCompare());

      return experiment_results;
    }

  }
}
