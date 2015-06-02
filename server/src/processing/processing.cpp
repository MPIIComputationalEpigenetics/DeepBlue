//
//  processing.cpp
//  epidb
//
//  Created by Felipe Albrecht on 02.06.15.
//  Copyright (c) 2015 Max Planck Institute for Computer Science. All rights reserved.
//

#include <atomic>
#include <memory>
#include <stack>
#include <string>

#include "processing.hpp"

namespace epidb {

  namespace processing {

    Status::Status(const std::string &id) :
      request_id(id)
    { }

    void Status::start_operation(OP op)
    {
      time_t time_;
      time(&time_);
      OperationStatus current;
      current.op = op;
      current.start = time_;

      operations.push(std::move(current));
    }

    void Status::end_operation()
    {
      if (!operations.empty()) {
        time_t time_;
        time(&time_);
        operations.top().end = time_;
      }
    }

    void Status::sum_regions(size_t qtd)
    {
      total_regions += qtd;
    }

    void Status::sum_size(size_t size)
    {
      total_size += size;
    }

    typedef std::shared_ptr<Status> StatusPtr;

    StatusPtr build_status(const std::string& _id)
    {
      return std::shared_ptr<Status>(new Status(_id));
    }

    StatusPtr build_dummy_status()
    {
      return std::shared_ptr<Status>(new Status("DUMMY"));
    }

  }
}

