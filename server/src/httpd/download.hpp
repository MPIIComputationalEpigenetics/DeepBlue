//
//  download.hpp
//  epidb
//
//  Created by Felipe Albrecht on 16.06.15.
//  Copyright (c) 2015 Max Planck Institute for Computer Science. All rights reserved.
//

#ifndef EPIDB_HTTPD_DOWNLOAD_HPP
#define EPIDB_HTTPD_DOWNLOAD_HPP

#include <string>
#include <vector>
#include <boost/noncopyable.hpp>
#include <memory>

#include "../extras/serialize.hpp"

#include "../../third_party/expat/lib/expat.h"

#include "xmlrpc_request.hpp"

namespace epidb {
  namespace httpd {

    Reply get_download_data(const std::string& uri);

  } // namespace httpd
} // namespace epidb

#endif // EPIDB_HTTPD_DOWNLOAD_HPP
