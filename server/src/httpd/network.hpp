//
//  network.hpp
//  epidb
//
//  Created by Felipe Albrecht on 25.05.13.
//  Copyright (c) 2016 Max Planck Institute for Informatics. All rights reserved.
//

#ifndef EPIDB_HTTPD_NETWORK_HPP
#define EPIDB_HTTPD_NETWORK_HPP

#include <vector>
#include <memory>

namespace epidb {
  namespace httpd {

    typedef std::shared_ptr<std::vector<char> > vector_ptr;
    typedef std::shared_ptr<std::string> ContentPtr;

  } // namespace httpd
} // namespace epidb
#endif // EPIDB_HTTPD_NETWORK_HPP
