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

#include "../config/config.hpp"

#include "../datatypes/user.hpp"

#include "../dba/users.hpp"

#include "../extras/stringbuilder.hpp"
#include "../extras/utils.hpp"

#include "../mdbq/client.hpp"
#include "../mdbq/janitor.hpp"


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
      std::string user_id = o["user_id"].str();
      std::string msg;
      if (!dba::users::get_user_by_id(user_id, user, msg)) {
        finish(BSON("error" << msg), false);
      }

      processing::StatusPtr status = processing::build_status(id, user.memory_limit());
      mongo::BSONObj result;
      bool success = process(user, o, status, result);
      finish(result, success);
    }

    bool QueueHandler::process(const datatypes::User &user, const mongo::BSONObj &job, processing::StatusPtr status, mongo::BSONObj& result)
    {
      std::string command = job["command"].str();

      if (command == "count_regions") {
        return process_count(user, job["query_id"].str(), status, result);
      }
      if (command == "coverage") {
        return process_coverage(user, job["query_id"].str(), job["genome"].str(), status, result);
      }
      if (command == "get_regions") {
        return process_get_regions(user, job["query_id"].str(), job["format"].str(), status, result);
      }
      if (command == "score_matrix") {
        return process_score_matrix(user, job["experiments_formats"].Obj(), job["aggregation_function"].str(), job["query_id"].str(), status, result);
      }
      if (command == "get_experiments_by_query") {
        return process_get_experiments_by_query(user, job["query_id"].str(), status, result);
      }
      if (command == "binning") {
        return process_binning(user, job["query_id"].str(), job["column_name"].str(), job["bars"].Int(), status, result);
      }
      if (command == "distinct") {
        return process_distinct(user, job["query_id"].str(), job["column_name"].str(), status, result);
      }
      if (command == "calculate_enrichment") {
        return process_calculate_enrichment(user, job["query_id"].str(), job["gene_model"].str(),  status, result);
      }
      if (command == "lola") {
        return process_lola(user, job["query_id"].str(), job["universe_query_id"].str(), job["databases"].Obj(), job["genome"].str(), status, result);
      }

      else {
        mongo::BSONObjBuilder bob;      datatypes::User user;

        bob.append("__error__", "Invalid command " + command);
        result = BSON("__error__" << ("Invalid command " + command));
        return false;
      }
    }

    bool QueueHandler::process_count(const datatypes::User &user,
                                     const std::string &query_id,
                                     processing::StatusPtr status, mongo::BSONObj& result)
    {
      std::string msg;
      mongo::BSONObjBuilder bob;
      size_t count = 0;

      if (!processing::count_regions(user, query_id, status, count, msg)) {
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

    bool QueueHandler::process_binning(const datatypes::User &user,
                                       const std::string &query_id, const std::string& column_name, const int bars,

                                       processing::StatusPtr status, mongo::BSONObj& result)
    {
      std::string msg;
      mongo::BSONObjBuilder bob;
      mongo::BSONObj binning;

      if (!processing::binning(user, query_id, column_name, bars, status, binning, msg)) {
        bob.append("__error__", msg);
        result = bob.obj();
        return false;
      }

      int size = binning.objsize();
      bob.append("binning", binning);
      status->set_total_stored_data(size);
      status->set_total_stored_data_compressed(size);
      result = bob.obj();

      if (is_canceled(status, msg)) {
        return false;
      }

      return true;
    }

    bool QueueHandler::process_distinct(const datatypes::User &user,
                                        const std::string &query_id, const std::string& column_name,
                                        processing::StatusPtr status, mongo::BSONObj& result)
    {
      std::string msg;
      mongo::BSONObjBuilder bob;
      mongo::BSONObj distinct;

      if (!processing::distinct(user, query_id, column_name, status, distinct, msg)) {
        bob.append("__error__", msg);
        result = bob.obj();
        return false;
      }

      int size = distinct.objsize();
      bob.append("distinct", distinct);
      status->set_total_stored_data(size);
      status->set_total_stored_data_compressed(size);
      result = bob.obj();

      if (is_canceled(status, msg)) {
        return false;
      }

      return true;
    }

    bool QueueHandler::process_calculate_enrichment(const datatypes::User &user,
        const std::string &query_id, const std::string& gene_model,
        processing::StatusPtr status, mongo::BSONObj& result)
    {
      std::string msg;
      mongo::BSONObjBuilder bob;
      mongo::BSONObj enrichment;

      if (!processing::calculate_enrichment(user, query_id, gene_model, status, enrichment, msg)) {
        bob.append("__error__", msg);
        result = bob.obj();
        return false;
      }

      int size = enrichment.objsize();
      bob.append("enrichment", enrichment);
      status->set_total_stored_data(size);
      status->set_total_stored_data_compressed(size);
      result = bob.obj();

      if (is_canceled(status, msg)) {
        return false;
      }

      return true;
    }

    bool QueueHandler::process_lola(const datatypes::User &user,
                                    const std::string &query_id, const std::string &universe_query_id,
                                    const mongo::BSONObj& datasets,
                                    const std::string& genome,
                                    processing::StatusPtr status, mongo::BSONObj& result)
    {
      std::string msg;
      mongo::BSONObjBuilder bob;
      mongo::BSONObj enrichment;

      if (!processing::lola(user, query_id, universe_query_id, datasets, genome, status, enrichment, msg)) {
        bob.append("__error__", msg);
        result = bob.obj();
        return false;
      }

      int size = enrichment.objsize();
      bob.append("enrichment", enrichment);
      status->set_total_stored_data(size);
      status->set_total_stored_data_compressed(size);
      result = bob.obj();

      if (is_canceled(status, msg)) {
        return false;
      }

      return true;
    }


    bool QueueHandler::process_coverage(const datatypes::User &user,
                                        const std::string &query_id, const std::string &genome,
                                        processing::StatusPtr status, mongo::BSONObj& result)
    {
      std::string msg;
      mongo::BSONObjBuilder bob;
      std::vector<processing::CoverageInfo> coverage_infos;

      if (!processing::coverage(user, query_id, genome, status, coverage_infos, msg)) {
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

    bool QueueHandler::process_get_regions(const datatypes::User &user,
                                           const std::string &query_id, const std::string &format,

                                           processing::StatusPtr status, mongo::BSONObj& result)
    {

      std::string msg;
      StringBuilder sb;
      mongo::BSONObjBuilder bob;

      if (!processing::get_regions(user, query_id, format, status, sb, msg)) {
        bob.append("__error__", msg);
        result = bob.obj();
        return false;
      }

      status->start_operation(processing::BUILDING_OUTPUT,
                              BSON("string_builder_size" << (long long) sb.size()));

      std::string result_string = sb.to_string();

      status->start_operation(processing::COMPRESSING_OUTPUT,
                              BSON("string_size" << (long long) result_string.size()));
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

    bool QueueHandler::process_score_matrix(const datatypes::User &user,
                                            const mongo::BSONObj &experiments_formats_bson, const std::string &aggregation_function, const std::string &regions_query_id,
                                            processing::StatusPtr status, mongo::BSONObj& result)
    {
      std::string msg;
      StringBuilder sb;
      mongo::BSONObjBuilder bob;

      std::vector<std::pair<std::string, std::string>> experiments_formats;

      for ( auto i = experiments_formats_bson.begin(); i.more(); ) {
        mongo::BSONElement e = i.next();

        std::string experiment_name = e.fieldName();
        std::string columns_name = e.str();

        experiments_formats.emplace_back(experiment_name, columns_name);
      }

      std::string matrix;
      if (!processing::score_matrix(user, experiments_formats, aggregation_function, regions_query_id, status, matrix, msg)) {
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

    bool QueueHandler::process_get_experiments_by_query(const datatypes::User &user,
        const std::string &query_id,
        processing::StatusPtr status, mongo::BSONObj& result)
    {
      std::string msg;
      mongo::BSONObjBuilder bob;
      std::vector<utils::IdName> experiments;
      if (!processing::get_experiments_by_query(user, query_id, status, experiments, msg)) {
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

    bool QueueHandler::is_canceled(processing::StatusPtr status, std::string& msg)
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

      std::string server = config::get_mongodb_server();
      std::string collection = config::DATABASE_NAME();

      QueueHandler *clients[num];
      boost::thread   *threads[num];

      for (size_t i = 0; i < num; ++i) {
        clients[i] = new QueueHandler(i, server, collection);
        threads[i] = new boost::thread(boost::bind(&QueueHandler::run, clients[i]));
      }

      mdbq::Janitor* janitor = new mdbq::Janitor();

      config::get_config_subject()->attachObserver(janitor);
      boost::thread *t = new boost::thread(boost::bind(&mdbq::Janitor::run, janitor));
    }
  }
}