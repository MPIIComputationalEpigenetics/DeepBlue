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

#include "../extras/utils.hpp"

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
      std::string command = job["command"].str();

      if (command == "count_regions") {
        return process_count(job["query_id"].str(), job["user_key"].str());
      } if (command == "get_regions") {
        return process_get_regions(job["query_id"].str(), job["format"].str(), job["user_key"].str());
      } if (command == "score_matrix") {
        return process_score_matrix(job["experiments_formats"].Obj(), job["aggregation_function"].str(), job["regions_query_id"].str(), job["user_key"].str());
      } if (command == "get_experiments_by_query") {
        return process_get_experiments_by_query(job["query_id"].str(), job["user_key"].str());
      } else {
        mongo::BSONObjBuilder bob;
        bob.append("success", false);
        bob.append("__error__", "Invalid command " + command);
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

    mongo::BSONObj QueueHandler::process_score_matrix(const mongo::BSONObj &experiments_formats_bson, const std::string &aggregation_function, const std::string &regions_query_id, const std::string &user_key)
    {
      std::string msg;
      StringBuilder sb;
      mongo::BSONObjBuilder bob;

      std::vector<std::pair<std::string, std::string>> experiments_formats;

      for ( mongo::BSONObj::iterator i = experiments_formats_bson.begin(); i.more(); ) {
        mongo::BSONElement e = i.next();

        std::string experiment_name = e.fieldName();
        std::string columns_name = e.str();

        experiments_formats.emplace_back(experiment_name, columns_name);
      }

      std::string matrix;
      if (!processing::score_matrix(experiments_formats, aggregation_function, regions_query_id, user_key, matrix, msg)) {
        bob.append("__error__", msg);
        return bob.obj();
      }

      std::string filename = store_result(matrix.c_str(), matrix.length());
      bob.append("__file__", filename);

      return bob.obj();
    }

    mongo::BSONObj QueueHandler::process_get_experiments_by_query(const std::string &query_id, const std::string &user_key)
    {
      std::string msg;
      mongo::BSONObjBuilder bob;
      std::vector<utils::IdName> experiments;
      if (!processing::get_experiments_by_query(query_id, user_key, experiments, msg)) {
        bob.append("__error__", msg);
        return bob.obj();
      }

      std::cerr << experiments.size() << std::endl;

      mongo::BSONObjBuilder experiments_ids_bob;
      for (auto &exp_format : experiments) {
        std::cerr << exp_format.id << std::endl;
        experiments_ids_bob.appendElements(BSON(exp_format.id << exp_format.name));
      }

      bob.append("__id_names__", experiments_ids_bob.obj());

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