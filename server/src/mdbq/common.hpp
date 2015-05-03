//
//  common.hpp
//  epidb
//
//  Created by Felipe Albrecht on 22.01.15.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//
// From: https://github.com/temporaer/MDBQ/

#ifndef __MDBQ_COMMON_HPP__
#define __MDBQ_COMMON_HPP__

namespace mdbq {
  enum TaskState {
    TS_NEW,
    TS_RUNNING,
    TS_DONE,
    TS_FAILED,
    _TS_END,
    _TS_FIRST = TS_NEW
  };
}
#endif /* __MDBQ_COMMON_HPP__ */
