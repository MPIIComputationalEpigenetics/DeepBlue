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

#include <boost/program_options.hpp>

#include <mongo/client/init.h>

#include "log.hpp"
#include "version.hpp"
#include "dba/config.hpp"
#include "engine/queue_processer.hpp"
#include "extras/compress.hpp"
#include "httpd/server.hpp"

#include "parser/wig.hpp"

int main(int argc, char *argv[])
{
  EPIDB_LOG(epidb::Version::info());

  namespace po = boost::program_options;

  std::string address;
  std::string port;
  size_t threads;
  size_t processing_threads;
  std::string mongodb_server;
  std::string database_name;
  long long processing_max_memory;

  // Declare the supported options.
  po::options_description desc("DeepBlue parameters");
  desc.add_options()
  ("help,H", "Help message")
  ("address,A", po::value<std::string>(&address)->default_value("localhost"), "Local address")
  ("port,P", po::value<std::string>(&port)->default_value("31415"), "Local port")
  ("threads,T", po::value<size_t>(&threads)->default_value(10), "Number of concurrent http data listeners")
  ("mongodb,M", po::value<std::string>(&mongodb_server)->default_value("mongodb://localhost:27017"), "MongoDB address and port")
  ("database_name,D", po::value<std::string>(&database_name)->default_value("epidb"), "Database name")
  ("processing_threads,R", po::value<size_t>(&processing_threads)->default_value(4), "Number of concurrent threads for processing request data")
  ("processing_max_memory,O", po::value<long long>(&processing_max_memory)->default_value(8ll * 1024 * 1024 * 1024), "Maximum memory available for request data processing (in bytes) ")
  ("sharding,S", "Use DeepBlue with sharding in the MongoDB")
  ;

  po::variables_map vm;
  try {
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
  } catch (const boost::program_options::error &e) {
    EPIDB_LOG_ERR(e.what());
    return 1;
  }

  if (vm.count("help")) {
    std::cout << desc << "\n";
    return 1;
  }

  EPIDB_LOG("Executing DeepBlue at " << address << ":" << port << " with " << threads << " threads.");

  EPIDB_LOG("Initializing MongoDB Client...");
  mongo::client::Options op; // weird ugly core dump if I do not create this object here
  mongo::client::initialize(op);
  // TODO: check initialize output
  EPIDB_LOG("MongoDB Client initialized.");

  EPIDB_LOG("Connecting to MongoDB server " << mongodb_server << " and using database " << database_name << ".")

  epidb::dba::config::set_sharding(vm.count("sharding"));
  epidb::dba::config::set_mongodb_server(mongodb_server);
  epidb::dba::config::set_database_name(database_name);
  epidb::dba::config::set_processing_max_memory(processing_max_memory);

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
  if (!epidb::compress::init()) {
    EPIDB_LOG_ERR("Problem initializing LZO compression algorithm.");
    return 1;
  }

  epidb::engine::queue_processer_run(processing_threads);

  epidb::httpd::server s(address, port, threads);
  s.run();

  return 0;
}
