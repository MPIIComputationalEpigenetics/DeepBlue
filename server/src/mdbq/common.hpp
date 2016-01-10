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
      TS_NEW,
      TS_RUNNING,
      TS_DONE,
      TS_FAILED,
      TS_CANCELLED,
      TS_REMOVED,
      TS_RENEW,
      _TS_END,
      _TS_FIRST = TS_NEW
    };
  }
}
#endif /* __MDBQ_COMMON_HPP__ */
