//
//  queue_processer.cpp
//  epidb
//
//  Created by Felipe Albrecht on 22.01.15.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include <boost/asio.hpp>
#include <boost/thread.hpp>

#include <mongo/client/dbclient.h>

#include "../dba/config.hpp"

#include "../engine/engine.hpp"

#include "../mdbq/client.hpp"

namespace epidb {
  namespace engine {
    class QueueHandler : public mdbq::Client {
    public:
      boost::asio::io_service ios;
      size_t _id;
      QueueHandler(size_t id, std::string &url, std::string &prefix) :
        mdbq::Client(url, prefix),
        _id(id) {}

      void handle_task(const mongo::BSONObj &o)
      {
        try {
          mongo::BSONObj result = epidb::Engine::instance().process(o);
          std::cerr << result.toString() << std::endl;
          finish(result, true);
        } catch (mdbq::timeout_exception) {
          /* do nothing */
        }
      }

      void run()
      {
        std::cerr << "run()" << std::endl;
        this->reg(ios, 1);
        ios.run();
      }

    };


    void queue_processer_run(size_t num)
    {
      boost::asio::io_service io;

      std::cerr << "queue_processer_run(" << num << ")" << std::endl;

      std::string server = dba::config::get_mongodb_server();
      std::string collection = dba::config::DATABASE_NAME() + "_queue";

      QueueHandler *clients[num];
      boost::thread   *threads[num];

      for (size_t i = 0; i < num; ++i) {
        clients[i] = new QueueHandler(i, server, collection);
        threads[i] = new boost::thread(boost::bind(&QueueHandler::run, clients[i]));
      }

    }
  }
}