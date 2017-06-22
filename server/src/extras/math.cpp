//
//  math.cpp
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
#include <boost/math/distributions/hypergeometric.hpp>
using namespace boost::math;

namespace epidb {
  namespace math {
    double fisher_test(unsigned a, unsigned b, unsigned c, unsigned d)
    {
      unsigned N = a + b + c + d;
      unsigned r = a + c;
      unsigned n = c + d;
      unsigned max_for_k = std::min(r, n);
      unsigned min_for_k = (unsigned) std::max(0, int(r + n - N));
      hypergeometric_distribution<> hgd(r, n, N);
      double cutoff = pdf(hgd, c);
      double tmp_p = 0.0;
      for(unsigned k = min_for_k; k < max_for_k + 1; k++) {
        double p = pdf(hgd, k);
        if(p <= cutoff) tmp_p += p;
      }
      return tmp_p;
    }
  }
}
