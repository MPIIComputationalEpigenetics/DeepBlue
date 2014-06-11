//
//  levenshtein.cpp
//  epidb
//
//  Created by Felipe Albrecht on 23.06.13.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include <algorithm>
#include <string>
#include <limits>
#include <vector>

#include <boost/foreach.hpp>

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
      int I_i[N_a + 1][N_b + 1], I_j[N_a + 1][N_b + 1]; // Index matrices to remember the 'path' for backtracking

      // here comes the actual algorithm
      for (int i = 1; i <= N_a; i++) {
        for (int j = 1; j <= N_b; j++) {
          temp[0] = H[i - 1][j - 1] + similarity_score(seq_a[i - 1], seq_b[j - 1], mu);
          temp[1] = H[i - 1][j] - delta;
          temp[2] = H[i][j - 1] - delta;
          temp[3] = 0.;
          H[i][j] = find_array_max(temp, 4, ind);
          switch (ind) {
          case 0:                                  // score in (i,j) stems from a match/mismatch
            I_i[i][j] = i - 1;
            I_j[i][j] = j - 1;
            break;
          case 1:                                  // score in (i,j) stems from a deletion in sequence A
            I_i[i][j] = i - 1;
            I_j[i][j] = j;
            break;
          case 2:                                  // score in (i,j) stems from a deletion in sequence B
            I_i[i][j] = i;
            I_j[i][j] = j - 1;
            break;
          case 3:                                  // (i,j) is the beginning of a subsequence
            I_i[i][j] = i;
            I_j[i][j] = j;
            break;
          }
        }
      }

      // search H for the maximal score
      double H_max = 0.;
      int i_max = 0, j_max = 0;
      for (int i = 1; i <= N_a; i++) {
        for (int j = 1; j <= N_b; j++) {
          if (H[i][j] > H_max) {
            H_max = H[i][j];
            i_max = i;
            j_max = j;
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

      BOOST_FOREACH(std::string tm, tms) {
        float value = calculate_score(term, tm);
        std::pair<std::string, float> score_pair(tm, value);
        ps.push_back(score_pair);
      }

      std::sort (ps.begin(), ps.end(), comp);

      std::vector<std::string> names;

      for (std::vector<std::pair<std::string, float> >::iterator it = ps.begin(); it != ps.end() && it->second > threshould; it++) {
        names.push_back(it->first);
      }

      return names;
    }
  }
}
