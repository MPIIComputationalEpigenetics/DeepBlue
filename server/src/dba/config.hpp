//
//  config.hpp
//  epidb
//
//  Created by Felipe Albrecht on 09.07.13.
//  Copyright (c) 2016 Max Planck Institute for Informatics. All rights reserved.
//

#ifndef EPIDB_DBA_CONFIG_HPP
#define EPIDB_DBA_CONFIG_HPP

#include <string>
#include <vector>

namespace mongo {
  class ConnectionString;
}

namespace epidb {
  namespace dba {

    namespace config {

      extern std::string mongodb_server;
      extern mongo::ConnectionString mongodb_server_connection;
      extern std::string database_name;
      extern long long processing_max_memory;
      extern std::vector<std::string> shards;

      bool set_shards_tags();

      void set_mongodb_server(const std::string &server);
      const std::string get_mongodb_server();
      const mongo::ConnectionString get_mongodb_server_connection();

      bool sharding();
      void set_sharding(bool sharding);
      std::vector<std::string> get_shards_names();
      void set_chunk_size(size_t);
      bool check_mongodb(std::string &msg);
      void set_database_name(const std::string &name);
      const std::string DATABASE_NAME();
      void set_processing_max_memory(const long long memory);
      long long get_processing_max_memory();
    }
  }
}

#endif /* defined(EPIDB_DBA_CONFIG_HPP) */