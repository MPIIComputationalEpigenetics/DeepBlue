//
//  config.cpp
//  epidb
//
//  Created by Fabian Reinartz on 11.03.2014
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include <iostream>
#include <string>
#include <vector>

#include <mongo/bson/bson.h>
#include <mongo/client/dbclient.h>

#include "config.hpp"

#include "../log.hpp"

namespace epidb {
  namespace dba {

    namespace config {

      std::vector<std::string> shards;

      std::string MONGODB_SERVER("localhost:27017");

      void set_mongodb_server(const std::string &server)
      {
        MONGODB_SERVER = server;
      }

      const std::string get_mongodb_server()
      {
        return MONGODB_SERVER;
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

      size_t chunk_size_value = 4;
      size_t chunk_size()
      {
        return chunk_size_value;
      }

      void set_chunk_size(size_t size)
      {
        chunk_size_value = size;
      }

      bool set_shards_tags()
      {
        mongo::ScopedDbConnection c(config::get_mongodb_server());


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
        }

        c.done();
        return true;

      }

    }
  }
}
