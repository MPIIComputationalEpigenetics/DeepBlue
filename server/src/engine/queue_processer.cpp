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

#include "../extras/stringbuilder.hpp"

#include "../mdbq/client.hpp"

#include "../processing/processing.hpp"

#include "queue_processer.hpp"

namespace epidb {
  namespace engine {

    QueueHandler::QueueHandler(size_t id, std::string &url, std::string &prefix) :
      mdbq::Client(url, prefix),
      _id(id) {}

    void QueueHandler::handle_task(const mongo::BSONObj &o)
    {
      try {
        mongo::BSONObj result = process(o);
        std::cerr << result.toString() << std::endl;
        finish(result, true);
      } catch (mdbq::timeout_exception) {
        /* do nothing */
      }
    }

    void QueueHandler::run()
    {
      std::cerr << "run()" << std::endl;
      this->reg(ios, 0.1);
      ios.run();
    }

    mongo::BSONObj QueueHandler::process(const mongo::BSONObj &job)
    {
      std::cerr << job.toString() << std::endl;
      std::string command = job["command"].str();

      if (command == "count_regions") {
        return process_count(job["query_id"].str(), job["user_key"].str());
      } if (command == "get_regions") {
        return process_get_regions(job["query_id"].str(), job["format"].str(), job["user_key"].str());
      } else {
        mongo::BSONObjBuilder bob;
        bob.append("success", false);
        bob.append("__error__", "Invalid command" + command);
        return bob.obj();
      }
    }

    mongo::BSONObj QueueHandler::process_count(const std::string &query_id, const std::string &user_key)
    {
      std::string msg;
      mongo::BSONObjBuilder bob;
      size_t count = 0;

      if (!processing::count_regions(query_id, user_key, count, msg)) {
        bob.append("__error__", msg);
        return bob.obj();
      }

      bob.append("count", (long long) count);

      return bob.obj();
    }


    mongo::BSONObj QueueHandler::process_get_regions(const std::string &query_id, const std::string &format, const std::string &user_key)
    {
      std::string msg;
      StringBuilder sb;
      mongo::BSONObjBuilder bob;

      if (!processing::get_regions(query_id, format, user_key, sb, msg)) {
        bob.append("__error__", msg);
        return bob.obj();
      }

      std::string result = sb.to_string();
      std::string filename = store_result(result.c_str(), result.length());

      bob.append("__file__", filename);

      return bob.obj();
    }

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