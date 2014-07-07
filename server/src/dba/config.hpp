//
//  config.hpp
//  epidb
//
//  Created by Felipe Albrecht on 09.07.13.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#ifndef EPIDB_DBA_CONFIG_HPP
#define EPIDB_DBA_CONFIG_HPP

#include <string>
#include <vector>

namespace epidb {
  namespace dba {

    namespace config {
      bool set_shards_tags();
      extern std::vector<std::string> shards;
      void set_mongodb_server(const std::string &server);
      const std::string get_mongodb_server();
      bool sharding();
      void set_sharding(bool sharding);
      std::vector<std::string> get_shards_names();
      size_t chunk_size();
      void set_chunk_size(size_t);
      bool check_mongodb(std::string &msg);
      void set_database_name(const std::string &name);
      const std::string DATABASE_NAME();
    }
  }
}

#endif /* defined(EPIDB_DBA_CONFIG_HPP) */