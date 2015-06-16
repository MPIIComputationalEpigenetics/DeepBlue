//
//  config.cpp
//  epidb
//
//  Created by Fabian Reinartz on 11.03.2014
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include <string>
#include <vector>

#include <mongo/bson/bson.h>

#include "../connection/connection.hpp"

#include "config.hpp"

#include "dba.hpp"

#include "../errors.hpp"
#include "../log.hpp"

namespace epidb {
  namespace dba {

    namespace config {

      std::vector<std::string> shards;
      std::string mongodb_server;
      std::string database_name;
      mongo::ConnectionString mongodb_server_connection;
      long long processing_max_memory;

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
        //std::cerr <<  mongodb_server_connection << std::endl;
        return mongodb_server_connection;
      }

      const std::string get_mongodb_server()
      {
        //std::cerr <<  mongodb_server << std::endl;
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

      long long get_processing_max_memory()
      {
        return processing_max_memory;
      }

      const std::string DATABASE_NAME()
      {
        return database_name;
      }

      bool use_sharding = true;
      bool sharding()
      {
        return use_sharding;
      }

      void set_sharding(bool value)
      {
        use_sharding = value;
      }

      bool check_mongodb(std::string &msg)
      {
        try {
          bool init = is_initialized();
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

      bool set_shards_tags()
      {
        Connection c;

        bool b(false);

        mongo::Query q = mongo::Query().sort("_id");
        std::auto_ptr<mongo::DBClientCursor> cursor  = c->query("config.shards", q);
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

      std::vector<std::string> get_shards_names()
      {
        return shards;
      }
    }
  }
}
