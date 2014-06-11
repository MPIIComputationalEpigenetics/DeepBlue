//
//  main.cpp
//  epidb
//
//  Created by Felipe Albrecht on 27.05.13.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//
//
// based on:
//
// main.cpp
// ~~~~~~~~
//
// Copyright (c) 2003-2012 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <iostream>
#include <string>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/asio.hpp>

#include "httpd/server.hpp"
#include "dba/config.hpp"
#include "log.hpp"
#include "version.hpp"

int main(int argc, char *argv[])
{
  EPIDB_LOG(epidb::Version::info());

  if (argc < 5) {
    EPIDB_LOG_ERR("Usage: epidb <address> <port> <threads> <mongodb_server> [--nosharding]\n  For IPv4, try:\n  server 0.0.0.0 31415 1 127.0.0.1:27017\n  For IPv6, try:\n  server 0::0 31415 1 127.0.0.1:27017");
    return 1;
  }

  std::size_t num_threads = boost::lexical_cast<std::size_t>(argv[3]);
  epidb::httpd::server s(argv[1], argv[2], num_threads);

  std::string mongodb_server = argv[4];

  epidb::dba::config::set_mongodb_server(mongodb_server);

  if (argc == 6) {
    if (std::string(argv[5]) == "--nosharding") {

      EPIDB_LOG("No sharding");
      epidb::dba::config::set_sharding(false);
    } else {
      EPIDB_LOG_ERR("Unkown option " << argv[5]);
      return 1;
    }
  }

  if (epidb::dba::config::sharding()) {
    EPIDB_LOG("Configuring MongoDB Sharding [" << std::string(mongodb_server) << "]");
    epidb::dba::config::set_shards_tags();
  }

  EPIDB_LOG("Starting DeepBlue Epigenomics Data Server ");
  s.run();

  return 0;
}
