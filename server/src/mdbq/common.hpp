//
//  common.hpp
//  epidb
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
      _TS_END,
      _TS_FIRST = TS_NEW
    };
  }
}
#endif /* __MDBQ_COMMON_HPP__ */
