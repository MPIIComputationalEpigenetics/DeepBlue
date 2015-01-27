//
//  request_status.cpp
//  epidb
//
//  Created by Felipe Albrecht on 27.01.15.
//  Copyright (c) 2013,2014,2015 Max Planck Institute for Computer Science. All rights reserved.
//

#ifndef REQUEST_STATUS_HPP
#define REQUEST_STATUS_HPP

namespace epidb {
  namespace request {
    typedef struct {
      std::string state;
      std::string message;
    } Status;
  }
}

#endif