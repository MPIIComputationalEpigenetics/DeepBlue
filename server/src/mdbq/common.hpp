//
//  common.hpp
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
// Code from https://github.com/temporaer/MDBQ/
// adapted to DeepBlue by Felipe Albrecht on 22.01.2015

#ifndef __MDBQ_COMMON_HPP__
#define __MDBQ_COMMON_HPP__

namespace epidb {
  namespace mdbq {
    enum TaskState {
      TS_NEW,       // 0
      TS_RUNNING,   // 1
      TS_DONE,      // 2
      TS_FAILED,    // 3
      TS_CANCELLED, // 4
      TS_REMOVED,   // 5
      TS_RENEW,     // 6
      TS_CLEARED,   // 7
      TS_REPROCESS, // 8
      _TS_END,
      _TS_FIRST = TS_NEW
    };
  }
}
#endif /* __MDBQ_COMMON_HPP__ */
