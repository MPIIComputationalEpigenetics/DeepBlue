//
//  config.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Fabian Reinartz on 11.03.2014
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

#include <string>
#include <vector>

#include <mongo/bson/bson.h>

#include "../config/config.hpp"

#include "../connection/connection.hpp"

#include "../dba/dba.hpp"


#include "config_observer.hpp"
#include "errors.hpp"
#include "log.hpp"

namespace epidb {
  namespace config {

    bool use_sharding = true;
    std::vector<std::string> shards;
    std::string mongodb_server;
    std::string database_name;
    mongo::ConnectionString mongodb_server_connection;
    long long processing_max_memory;
    unsigned long long old_request_age_in_sec;

    std::shared_ptr<ConfigSubject> config_subject = std::make_shared<ConfigSubject>();

    ConfigSubjectPtr get_config_subject()
    {
      return config_subject;
    }

    void set_mongodb_server(const std::string &server)
    {
      std::cerr << "server: '" << server << "'" << std::endl;
      static std::string MONGODB_ADDR("mongodb://");
      static size_t SIZE = MONGODB_ADDR.size();

      if ((server.size() < SIZE) ||
          server.compare(0, SIZE, MONGODB_ADDR)) {
        mongodb_server = MONGODB_ADDR + server;
      } else {
        mongodb_server = server;
      }
      std::cerr << "mongodb_server: '" << mongodb_server << "'" << std::endl;

      std::string errMessage;
      mongodb_server_connection = mongo::ConnectionString::parse(mongodb_server, errMessage);

      std::cerr << "errMessage: " << errMessage << std::endl;
    }

    const mongo::ConnectionString get_mongodb_server_connection()
    {
      return mongodb_server_connection;
    }

    const std::string get_mongodb_server()
    {
      return mongodb_server;
    }

    void set_database_name(const std::string &name)
    {
      database_name = name;
    }

    void set_processing_max_memory(const long long memory)
    {
      processing_max_memory = memory;
    }

    bool check_mongodb(std::string &msg)
    {
      try {
        bool init = dba::is_initialized();
        if (init) {
          EPIDB_LOG_DBG("Database " << database_name << " is ready.");
        } else {
          EPIDB_LOG_DBG("Database " << database_name << " is not initialized.");
        }
        return true;
      } catch (const std::exception &e) {
        msg = Error::m(ERR_DATABASE_CONNECTION, e.what());
        return false;
      }
    }

    long long get_processing_max_memory()
    {
      return processing_max_memory;
    }

    unsigned long long get_old_request_age_in_sec()
    {
      return old_request_age_in_sec;
    }

    void set_old_request_age_in_sec(const unsigned long long oo)
    {
      old_request_age_in_sec = oo;
      config_subject->notifyObservers();
    }

    const std::string DATABASE_NAME()
    {
      return database_name;
    }

    bool set_shards_tags()
    {
      Connection c;

      bool b(false);

      mongo::Query q = mongo::Query().sort("_id");
      auto cursor  = c->query("config.shards", q);
      while  (cursor->more()) {
        mongo::BSONObj o = cursor->next();
        std::string shard = o["_id"].str();
        shards.push_back(shard);

        EPIDB_LOG_DBG("Setting shard tag for " << shard);

        mongo::BSONObj info;
        mongo::BSONObj findandmodify = BSON("findandmodify" << "shards" <<
                                            "query" << BSON("_id" << shard) <<
                                            "update" << BSON("$addToSet" << BSON("tags" << shard)));

        if (!c->runCommand("config", findandmodify, info)) {
          EPIDB_LOG_ERR("Problem while setting shard tag ' << " << shard << " for shard " << shard << ":" << info.toString());
          return false;
        }

        EPIDB_LOG_DBG("Shard tag: " << shard << " set" );
        b = true;
      }

      if (!b) {
        EPIDB_LOG_WARN("No shards were found in the MongoDB instance [" << config::get_mongodb_server() << "]");
      }

      c.done();
      return true;
    }

    bool sharding()
    {
      return use_sharding;
    }

    void set_sharding(bool value)
    {
      use_sharding = value;
    }

    std::vector<std::string> get_shards_names()
    {
      return shards;
    }
  }
}
