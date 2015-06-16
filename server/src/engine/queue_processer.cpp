//
//  queue_processer.cpp
//  epidb
//
//  Created by Felipe Albrecht on 22.01.15.
//  Copyright (c) 2013,2014,2015 Max Planck Institute for Computer Science. All rights reserved.
//

#include <iostream>
#include <regex>
#include <string>
#include <sstream>

#include <boost/asio.hpp>
#include <boost/thread.hpp>

#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/bzip2.hpp>
#include <boost/iostreams/stream.hpp>

#include <mongo/client/dbclient.h>

#include "../dba/config.hpp"

#include "../extras/stringbuilder.hpp"
#include "../extras/utils.hpp"

#include "../mdbq/client.hpp"

#include "../processing/processing.hpp"

#include "../log.hpp"

#include "queue_processer.hpp"

namespace epidb {
  namespace engine {

    QueueHandler::QueueHandler(size_t id, std::string &url, std::string &prefix) :
      mdbq::Client(url, prefix),
      _id(id) {}

    void QueueHandler::handle_task(const std::string& id, const mongo::BSONObj &o)
    {
      try {
        processing::StatusPtr status = processing::build_status(id, dba::config::get_processing_max_memory()); // TODO: get from user.
        mongo::BSONObj result = process(o, status);
        finish(result, true);
      } catch (mdbq::timeout_exception) {
        /* do nothing */
      }
    }

    void QueueHandler::run()
    {
      EPIDB_LOG_TRACE("Starting QueueHandler - " << utils::integer_to_string(_id));
      this->reg(ios, 0.1);
      ios.run();
    }

    mongo::BSONObj QueueHandler::process(const mongo::BSONObj &job, processing::StatusPtr status)
    {
      std::string _id = job["_id"];
      std::string command = job["command"].str();
      std::string user_key = job["user_key"].str();

      if (command == "count_regions") {
        return process_count(job["query_id"].str(), user_key, status);
      } if (command == "get_regions") {
        return process_get_regions(job["query_id"].str(), job["format"].str(), user_key, status);
      } if (command == "score_matrix") {
        return process_score_matrix(job["experiments_formats"].Obj(), job["aggregation_function"].str(), job["query_id"].str(), user_key, status);
      } if (command == "get_experiments_by_query") {
        return process_get_experiments_by_query(job["query_id"].str(), user_key, status);
      } else {
        mongo::BSONObjBuilder bob;
        bob.append("success", false);
        bob.append("__error__", "Invalid command " + command);
        return bob.obj();
      }
    }

    mongo::BSONObj QueueHandler::process_count(const std::string &query_id, const std::string &user_key, processing::StatusPtr status)
    {
      std::string msg;
      mongo::BSONObjBuilder bob;
      size_t count = 0;

      if (!processing::count_regions(query_id, user_key, status, count, msg)) {
        bob.append("__error__", msg);
        return bob.obj();
      }

      bob.append("count", (long long) count);
      status->set_total_stored_data(sizeof(long long));
      status->set_total_stored_data_compressed(sizeof(long long));

      return bob.obj();
    }


    mongo::BSONObj QueueHandler::process_get_regions(const std::string &query_id, const std::string &format, const std::string &user_key, processing::StatusPtr status)
    {
      std::string msg;
      StringBuilder sb;
      mongo::BSONObjBuilder bob;

      if (!processing::get_regions(query_id, format, user_key, status, sb, msg)) {
        bob.append("__error__", msg);
        return bob.obj();
      }

      std::string result = sb.to_string();

      std::stringbuf inStream(std::move(result));
      std::stringbuf outStream;
      boost::iostreams::filtering_streambuf< boost::iostreams::input> in;
      in.push( boost::iostreams::bzip2_compressor());
      in.push( inStream );
      boost::iostreams::copy(in, outStream);

      std::string compressed_s = outStream.str();
      const char* compressed = compressed_s.data();

      std::string filename = store_result(compressed, compressed_s.size());
      bob.append("__file__", filename);
      bob.append("__original_size__", (long long) result.size());
      bob.append("__compressed_size__", (long long) compressed_s.size());

      status->set_total_stored_data(result.size());
      status->set_total_stored_data_compressed(compressed_s.size());

      return bob.obj();
    }

    mongo::BSONObj QueueHandler::process_score_matrix(const mongo::BSONObj &experiments_formats_bson, const std::string &aggregation_function, const std::string &regions_query_id, const std::string &user_key, processing::StatusPtr status)
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
      if (!processing::score_matrix(experiments_formats, aggregation_function, regions_query_id, user_key, status, matrix, msg)) {
        bob.append("__error__", msg);
        return bob.obj();
      }

      std::stringbuf inStream(std::move(matrix));
      std::stringbuf outStream;
      boost::iostreams::filtering_streambuf< boost::iostreams::input> in;
      in.push( boost::iostreams::bzip2_compressor());
      in.push( inStream );
      boost::iostreams::copy(in, outStream);

      std::string compressed_s = outStream.str();
      const char* compressed = compressed_s.data();

      std::string filename = store_result(compressed, compressed_s.size());
      bob.append("__file__", filename);
      bob.append("__original_size__", (long long) matrix.size());
      bob.append("__compressed_size__", (long long) compressed_s.size());

      status->set_total_stored_data(matrix.size());
      status->set_total_stored_data_compressed(compressed_s.size());

      return bob.obj();
    }

    mongo::BSONObj QueueHandler::process_get_experiments_by_query(const std::string &query_id, const std::string &user_key, processing::StatusPtr status)
    {
      std::string msg;
      mongo::BSONObjBuilder bob;
      std::vector<utils::IdName> experiments;
      if (!processing::get_experiments_by_query(query_id, user_key, status, experiments, msg)) {
        bob.append("__error__", msg);
        return bob.obj();
      }

      mongo::BSONObjBuilder experiments_ids_bob;
      for (auto &exp_format : experiments) {
        std::cerr << exp_format.id << std::endl;
        experiments_ids_bob.appendElements(BSON(exp_format.id << exp_format.name));
      }

      mongo::BSONObj o = experiments_ids_bob.obj();
      bob.append("__id_names__", o);

      int size = o.objsize();
      status->set_total_stored_data(size);
      status->set_total_stored_data_compressed(size);
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