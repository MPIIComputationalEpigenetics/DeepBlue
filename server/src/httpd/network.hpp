//
//  network.hpp
//  epidb
//
//  Created by Felipe Albrecht on 25.05.13.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#ifndef EPIDB_HTTPD_NETWORK_HPP
#define EPIDB_HTTPD_NETWORK_HPP

#include <vector>
#include <boost/shared_ptr.hpp>

namespace epidb {
  namespace httpd {

    typedef boost::shared_ptr<std::vector<char> > vector_ptr;
    typedef boost::shared_ptr<std::string> ContentPtr;

  } // namespace httpd
} // namespace epidb
#endif // EPIDB_HTTPD_NETWORK_HPP
