//
//  levenshtein.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 23.06.13.
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

#include <algorithm>
#include <string>
#include <limits>
#include <vector>

#include "../extras/utils.hpp"

#include "levenshtein.hpp"

#define __min(a, b) (((a) < (b)) ? (a) : (b))
#define __max(a, b) (((a) > (b)) ? (a) : (b))

namespace epidb {
  namespace algorithms {

    double similarity_score(char a, char b, double mu)
    {
      double result;
      if (a == b) {
        result = 1.;
      } else {
        result = -mu;
      }
      return result;
    }


    double find_array_max(double array[], int length, int &ind)
    {
      double max = array[0];
      ind = 0;

      for (int i = 1; i < length; i++) {
        if (array[i] > max) {
          max = array[i];
          ind = i;
        }
      }
      return max;
    }

    // https://wiki.uni-koeln.de/biologicalphysics/index.php/Implementation_of_the_Smith-Waterman_local_alignment_algorithm
    template <class TokensCollection> static float levenshtein_distance(TokensCollection &seq_a, TokensCollection &seq_b)
    {
      double mu    = 0.25;
      double delta = 1;

      int ind;

      // string s_a=seq_a,s_b=seq_b;
      int N_a = seq_a.length();
      int N_b = seq_b.length();

      // initialize H
      double H[N_a + 1][N_b + 1];
      for (int i = 0; i <= N_a; i++) {
        for (int j = 0; j <= N_b; j++) {
          H[i][j] = 0.;
        }
      }

      double temp[4];
      double H_max = 0.;

      // here comes the actual algorithm
      for (int i = 1; i <= N_a; i++) {
        for (int j = 1; j <= N_b; j++) {
          temp[0] = H[i - 1][j - 1] + similarity_score(seq_a[i - 1], seq_b[j - 1], mu);
          temp[1] = H[i - 1][j] - delta;
          temp[2] = H[i][j - 1] - delta;
          temp[3] = 0.;
          H[i][j] = find_array_max(temp, 4, ind);

          if (H[i][j] > H_max) {
            H_max = H[i][j];
          }
        }
      }

      return H_max;
    }

    static bool comp( std::pair<std::string, float> p1, std::pair<std::string, float>p2)
    {
      return p1.second > p2.second;
    }

    float Levenshtein::calculate_score(const std::string &source,  const std::string &tm)
    {
      float metric;

      std::string norm_source = utils::normalize_name(source);
      std::string norm_tm = utils::normalize_name(tm);

      float dist;
      if (norm_tm.size() < norm_source.size()) {
        dist = levenshtein_distance(norm_source, norm_tm);
      } else {
        dist = levenshtein_distance(norm_tm, norm_source);
      }
      metric =  ((float) dist / (float) std::min(norm_tm.length(), norm_source.length()));

      return metric;
    }

    std::vector<std::string> Levenshtein::order_by_score(const std::string &term, const std::vector<std::string> &tms)
    {
      // TODO: put the threshould in the server config
      float threshould(0.60);
      std::vector<std::pair<std::string, float> > ps;

      for(const std::string & tm: tms) {
        float value = calculate_score(term, utils::normalize_name(tm));
        std::pair<std::string, float> score_pair(tm, value);
        ps.push_back(score_pair);
      }

      std::sort (ps.begin(), ps.end(), comp);

      std::vector<std::string> names;

      for (auto& v: ps) {
        if (v.second > threshould)
        names.push_back(v.first);
      }

      return names;
    }
  }
}
