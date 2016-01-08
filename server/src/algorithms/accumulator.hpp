//
//  accumulator.hpp
//  DeepBlue Epigenomic Data Server

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
//  Created by Felipe Albrecht on 11.11.14.
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

#ifndef EPIDB_ALGORITHMS_ACCUMULATOR_HPP
#define EPIDB_ALGORITHMS_ACCUMULATOR_HPP

#include <string>

#include "../types.hpp"

namespace epidb {
  namespace algorithms {

    class Accumulator {
    private:
      std::vector<Score> values;
      mutable bool _calculated;

      mutable Score _min;
      mutable Score _max;
      mutable Score _median;
      mutable Score _mean;
      mutable Score _var;
      mutable Score _sd;

      void calculate() const;

    public:
      Accumulator();
      void push(Score value);
      Score min() const;
      Score max() const;
      Score mean() const;
      Score var() const;
      Score sd() const;
      Score median() const;
      Score count() const;
      Score boolean() const;
      const std::string string(std::string sep) const;
    };

    typedef Score (Accumulator::*GetDataPtr)() const;
    GetDataPtr get_function_data(const std::string& function_name);
  }
}

#endif
