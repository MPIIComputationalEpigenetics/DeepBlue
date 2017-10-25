//
//  enrichment_result.hpp
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

#ifndef ENRICHMENT_RESULT_HPP
#define ENRICHMENT_RESULT_HPP

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
                     std::unordered_map<std::string, int>& datasets_odd_rank);

    std::vector<std::shared_ptr<ExperimentResult>> sort_results(
          const std::vector<ProcessOverlapResult>& results,
          std::vector<std::tuple<std::string, size_t>>& datasets_support,
          std::vector<std::tuple<std::string, double>>& datasets_log_score,
          std::vector<std::tuple<std::string, double>>& datasets_odds_score);
  }
}

#endif