//
//  config.hpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 09.07.13.
//  Copyright (c) 2016 Max Planck Institute for Informatics. All rights reserved.

//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.

//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.

//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#ifndef EPIDB_CONFIG_HPP
#define EPIDB_CONFIG_HPP

#include <string>
#include <vector>
#include <memory>

#include "config_observer.hpp"

namespace mongo {
  class ConnectionString;
}

namespace epidb {
  namespace config {

    typedef std::shared_ptr<ConfigSubject> ConfigSubjectPtr;

    ConfigSubjectPtr get_config_subject();

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
    unsigned long long get_old_request_age_in_sec();
    void set_old_request_age_in_sec(const unsigned long long oo);
    unsigned long long get_default_old_request_age_in_sec();
    void set_default_old_request_age_in_sec(const unsigned long long oo);
  }
}

#endif /* defined(EPIDB_DBA_CONFIG_HPP) */