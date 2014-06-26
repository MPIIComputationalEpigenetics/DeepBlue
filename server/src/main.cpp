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
#include <boost/program_options.hpp>

#include "httpd/server.hpp"
#include "dba/config.hpp"
#include "log.hpp"
#include "version.hpp"

int main(int argc, char *argv[])
{
  EPIDB_LOG(epidb::Version::info());

  namespace po = boost::program_options;

  std::string address;
  std::string port;
  size_t threads;
  std::string mongodb_server;
  std::string database_name;

  // Declare the supported options.
  po::options_description desc("DeepBlue Command Line Parameters");
  desc.add_options()
  ("help,H", "produce help message")
  ("address,A", po::value<std::string>(&address)->default_value("localhost"), "local address")
  ("port,P", po::value<std::string>(&port)->default_value("31415"), "local port")
  ("threads,T", po::value<size_t>(&threads)->default_value(10), "number of concurrent requests")
  ("mongodb,M", po::value<std::string>(&mongodb_server)->default_value("127.0.0.1:27017"), "MongoDB address and port")
  ("database_name,D", po::value<std::string>(&database_name)->default_value("epidb"), "Database Name")
  ("nosharding", "do not use sharding in the mongodb")
  ;

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cout << desc << "\n";
    return 1;
  }

  epidb::dba::config::set_sharding(!vm.count("nosharding"));
  epidb::dba::config::set_mongodb_server(mongodb_server);

  std::string msg;
  if (!epidb::dba::config::check_mongodb(msg)) {
    EPIDB_LOG_ERR(msg);
    return 1;
  }

  if (epidb::dba::config::sharding()) {
    EPIDB_LOG("Configuring MongoDB Sharding [" << std::string(mongodb_server) << "]");
    epidb::dba::config::set_shards_tags();
  }

  EPIDB_LOG("Starting DeepBlue Epigenomics Data Server ");
  epidb::httpd::server s(address, port, threads);
  s.run();

  return 0;
}
