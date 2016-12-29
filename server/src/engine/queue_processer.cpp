//
//  queue_processer.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 22.01.15.
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

#include "../datatypes/user.hpp"

#include "../dba/config.hpp"
#include "../dba/users.hpp"

#include "../extras/stringbuilder.hpp"
#include "../extras/utils.hpp"

#include "../mdbq/client.hpp"

#include "../processing/processing.hpp"

#include "../errors.hpp"
#include "../log.hpp"

#include "queue_processer.hpp"

namespace epidb {
  namespace engine {

    QueueHandler::QueueHandler(size_t id, std::string &url, std::string &prefix) :
      mdbq::Client(url, prefix),
      _id(id) {}

    void QueueHandler::run()
    {
      EPIDB_LOG_TRACE("Starting QueueHandler - " << utils::integer_to_string(_id));
      this->reg(ios, 1);
      ios.run();
    }

    void QueueHandler::handle_task(const std::string& id, const mongo::BSONObj &o)
    {
      datatypes::User user;
      std::string user_key = o["user_id"].str();
      std::string msg;
      if (!dba::users::get_user_by_id(user_key, user, msg)) {
        finish(BSON("error" << msg), false);
      }

      processing::StatusPtr status = processing::build_status(id, user.get_memory_limit());
      mongo::BSONObj result;
      bool success = process(o, status, result);
      finish(result, success);
    }

    bool QueueHandler::process(const mongo::BSONObj &job, processing::StatusPtr status, mongo::BSONObj& result)
    {
      std::string command = job["command"].str();
      std::string user_key = job["user_key"].str();

      if (command == "count_regions") {
        return process_count(job["query_id"].str(), user_key, status, result);
      } if (command == "coverage") {
        return process_coverage(job["query_id"].str(), job["genome"].str(), user_key, status, result);
      } if (command == "get_regions") {
        return process_get_regions(job["query_id"].str(), job["format"].str(), user_key, status, result);
      } if (command == "score_matrix") {
        return process_score_matrix(job["experiments_formats"].Obj(), job["aggregation_function"].str(), job["query_id"].str(), user_key, status, result);
      } if (command == "get_experiments_by_query") {
        return process_get_experiments_by_query(job["query_id"].str(), user_key, status, result);
      }  if (command == "get_experiments_by_query") {
        return process_histogram(job["query_id"].str(), job["column_name"].str(), job["bars"].Int(), user_key, status, result);
      } else {
        mongo::BSONObjBuilder bob;
        bob.append("__error__", "Invalid command " + command);
        result = BSON("__error__" << "Invalid command ");
        return false;
      }
    }

    bool QueueHandler::process_count(const std::string &query_id, const std::string &user_key, processing::StatusPtr status, mongo::BSONObj& result)
    {
      std::string msg;
      mongo::BSONObjBuilder bob;
      size_t count = 0;

      if (!processing::count_regions(query_id, user_key, status, count, msg)) {
        bob.append("__error__", msg);
        result = bob.obj();
        return false;
      }

      bob.append("count", (long long) count);
      status->set_total_stored_data(sizeof(long long));
      status->set_total_stored_data_compressed(sizeof(long long));
      result = bob.obj();

      if (is_canceled(status, msg)) {
        return false;
      }

      return true;
    }

    bool QueueHandler::process_histogram(const std::string &query_id, const std::string& column_name, const int bars, const std::string &user_key, processing::StatusPtr status, mongo::BSONObj& result)
    {
      std::string msg;
      mongo::BSONObjBuilder bob;
      mongo::BSONArray counts;

      if (!processing::histogram(query_id, column_name, bars, user_key, status, counts, msg)) {
        bob.append("__error__", msg);
        result = bob.obj();
        return false;
      }

      bob.append("count", counts);
      status->set_total_stored_data(sizeof(long long));
      status->set_total_stored_data_compressed(sizeof(long long));
      result = bob.obj();

      if (is_canceled(status, msg)) {
        return false;
      }

      return true;
    }

    bool QueueHandler::process_coverage(const std::string &query_id, const std::string &genome, const std::string &user_key, processing::StatusPtr status, mongo::BSONObj& result)
    {
      std::string msg;
      mongo::BSONObjBuilder bob;
      std::vector<processing::CoverageInfo> coverage_infos;

      if (!processing::coverage(query_id, genome, user_key, status, coverage_infos, msg)) {
        bob.append("__error__", msg);
        result = bob.obj();
        return false;
      }

      mongo::BSONObjBuilder coverages_bob;
      for (const auto &coverage_info : coverage_infos) {
        coverages_bob.append(coverage_info.chromosome_name,
                             BSON(
                               "size" << (long long) coverage_info.chromosome_size <<
                               "total" << (long long) coverage_info.total <<
                               "coverage" << (float) ((coverage_info.total * 100.0) / coverage_info.chromosome_size)
                             )
                            );
      }

      mongo::BSONObj o = coverages_bob.obj();
      int size = o.objsize();
      bob.append("coverages", o);
      status->set_total_stored_data(size);
      status->set_total_stored_data_compressed(size);
      result = bob.obj();

      if (is_canceled(status, msg)) {
        return false;
      }

      return true;
    }

    bool QueueHandler::process_get_regions(const std::string &query_id, const std::string &format, const std::string &user_key, processing::StatusPtr status, mongo::BSONObj& result)
    {
      std::string msg;
      StringBuilder sb;
      mongo::BSONObjBuilder bob;

      if (!processing::get_regions(query_id, format, user_key, status, sb, msg)) {
        bob.append("__error__", msg);
        result = bob.obj();
        return false;
      }

      std::string result_string = sb.to_string();

      std::stringbuf inStream(std::move(result_string));
      std::stringbuf outStream;
      boost::iostreams::filtering_streambuf< boost::iostreams::input> in;
      in.push( boost::iostreams::bzip2_compressor());
      in.push( inStream );
      boost::iostreams::copy(in, outStream);

      std::string compressed_s = outStream.str();
      const char* compressed = compressed_s.data();

      std::string filename = store_result(compressed, compressed_s.size());
      bob.append("__file__", filename);
      bob.append("__original_size__", (long long) result_string.size());
      bob.append("__compressed_size__", (long long) compressed_s.size());

      status->set_total_stored_data(result_string.size());
      status->set_total_stored_data_compressed(compressed_s.size());

      result = bob.obj();

      if (is_canceled(status, msg)) {
        return false;
      }

      return true;
    }

    bool QueueHandler::process_score_matrix(const mongo::BSONObj &experiments_formats_bson, const std::string &aggregation_function, const std::string &regions_query_id, const std::string &user_key, processing::StatusPtr status, mongo::BSONObj& result)
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
        result = bob.obj();
        return false;
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

      result = bob.obj();

      if (is_canceled(status, msg)) {
        return false;
      }

      return true;
    }

    bool QueueHandler::process_get_experiments_by_query(const std::string &query_id, const std::string &user_key, processing::StatusPtr status, mongo::BSONObj& result)
    {
      std::string msg;
      mongo::BSONObjBuilder bob;
      std::vector<utils::IdName> experiments;
      if (!processing::get_experiments_by_query(query_id, user_key, status, experiments, msg)) {
        bob.append("__error__", msg);
        result = bob.obj();
        return false;
      }

      mongo::BSONObjBuilder experiments_ids_bob;
      for (auto &exp_format : experiments) {
        experiments_ids_bob.appendElements(BSON(exp_format.id << exp_format.name));
      }

      mongo::BSONObj o = experiments_ids_bob.obj();
      bob.append("__id_names__", o);

      int size = o.objsize();
      status->set_total_stored_data(size);
      status->set_total_stored_data_compressed(size);
      result = bob.obj();

      if (is_canceled(status, msg)) {
        return false;
      }

      return true;
    }


    bool QueueHandler::is_canceled(processing::StatusPtr status, std::string msg)
    {
      bool is_canceled = false;
      if (!status->is_canceled(is_canceled, msg)) {
        return true;
      }
      if (is_canceled) {
        msg = Error::m(ERR_REQUEST_CANCELED);
        return true;
      }
      return false;
    }

    void queue_processer_run(size_t num)
    {
      boost::asio::io_service io;

      std::string server = dba::config::get_mongodb_server();
      std::string collection = dba::config::DATABASE_NAME();

      QueueHandler *clients[num];
      boost::thread   *threads[num];

      for (size_t i = 0; i < num; ++i) {
        clients[i] = new QueueHandler(i, server, collection);
        threads[i] = new boost::thread(boost::bind(&QueueHandler::run, clients[i]));
      }

    }
  }
}