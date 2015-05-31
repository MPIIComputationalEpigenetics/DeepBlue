//
//  engine.cpp
//  epidb
//
//  Created by Felipe Albrecht on 08.04.14.
//  Copyright (c) 2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include <sstream>
#include <vector>

#include "commands.hpp"
#include "engine.hpp"

#include "../dba/config.hpp"
#include "../dba/queries.hpp"
#include "../dba/users.hpp"

#include "../extras/compress.hpp"
#include "../extras/stringbuilder.hpp"
#include "../extras/utils.hpp"

#include "../processing/processing.hpp"

#include "../mdbq/common.hpp"

#include "../log.hpp"
#include "../version.hpp"

namespace epidb {

  Engine::Engine()
    : _hub(dba::config::get_mongodb_server(), dba::config::DATABASE_NAME() + "_queue")
  {
    EPIDB_LOG("Creating Engine");
  }

  bool Engine::init()
  {
    _hub.clear_all();

    return true;
  }

  bool Engine::execute(const std::string &name, const std::string &ip, unsigned long long id,
                       serialize::Parameters &parameters, serialize::Parameters &result) const
  {
    const Command *command = Command::get_command(name);
    if (command == 0) {
      std::stringstream ss;
      ss << "Command " << name << " does not exists.";
      EPIDB_LOG("Request (" << id << ") from " << ip << ": " << ss.str());
      result.add_error(ss.str());
      return false;
    }

    std::string msg;
    if (!command->check_parameters(parameters, msg)) {
      EPIDB_LOG("Request (" << id << ") from " << ip << ": " << name << " with bad typed parameters (" <<
                parameters.string(true) << "). " << msg);
      result.add_error(msg);
      return false;
    }

    EPIDB_LOG("Request (" << id << ") from " << ip << ": " << name << " with (" <<  parameters.string(true) << ").");
    return command->run(ip, parameters, result);
  }

  bool Engine::queue(const mongo::BSONObj &job, unsigned int timeout, std::string &id, std::string &msg)
  {
    if (!_hub.insert_job(job, timeout, Version::version_value(), id, msg)) {
      return false;
    }
    return true;
  }

  bool Engine::queue_count_regions(const std::string &query_id, const std::string &user_key, std::string &id, std::string &msg)
  {
    utils::IdName user;
    if (!dba::users::get_user(user_key, user, msg)) {
      return false;
    }
    if (!queue(BSON("command" << "count_regions" << "query_id" << query_id << "user_id" << user.id), 60 * 60, id, msg)) {
      return false;
    }
    return true;
  }

  bool Engine::queue_get_regions(const std::string &query_id, const std::string &output_format, const std::string &user_key, std::string &id, std::string &msg)
  {
    utils::IdName user;
    if (!dba::users::get_user(user_key, user, msg)) {
      return false;
    }
    if (!queue(BSON("command" << "get_regions" << "query_id" << query_id << "format" << output_format << "user_id" << user.id), 60 * 60, id, msg)) {
      return false;
    }
    return true;
  }

  bool Engine::queue_score_matrix(const std::vector<std::pair<std::string, std::string>> &experiments_formats, const std::string &aggregation_function, const std::string &regions_query_id, const std::string &user_key, std::string &id, std::string &msg)
  {
    mongo::BSONObjBuilder bob_formats;

    for (auto &exp_format : experiments_formats) {
      bob_formats.appendElements(BSON(exp_format.first << exp_format.second));
    }

    utils::IdName user;
    if (!dba::users::get_user(user_key, user, msg)) {
      return false;
    }

    if (!queue(BSON("command" << "score_matrix" << "experiments_formats" << bob_formats.obj() << "aggregation_function" << aggregation_function << "regions_query_id" << regions_query_id << "user_id" << user.id), 60 * 60, id, msg)) {
      return false;
    }

    return true;
  }


  bool Engine::queue_get_experiments_by_query(const std::string &query_id, const std::string &user_key, std::string &request_id, std::string &msg)
  {
    utils::IdName user;
    if (!dba::users::get_user(user_key, user, msg)) {
      return false;
    }
    if (!queue(BSON("command" << "get_experiments_by_query" << "query_id" << query_id << "user_id" << user.id), 60 * 60, request_id, msg)) {
      return false;
    }
    return true;
  }


  bool Engine::request_status(const std::string &request_id, const std::string &user_key, request::Status &request_status, std::string &msg)
  {
    utils::IdName user;
    if (!dba::users::get_user(user_key, user, msg)) {
      return false;
    }
    mongo::BSONObj o = _hub.get_job(request_id); // TODO still check
    if (o.isEmpty()) {
      msg = "Request ID " + request_id + " not found.";
      return false;
    }

    request_status.state = mdbq::Hub::state_name(o);
    request_status.message = mdbq::Hub::state_message(o);
    return true;
  }

  request::Job Engine::get_job_info(mongo::BSONObj o)
  {
    request::Job job;
    request::Status status;

    status.state = mdbq::Hub::state_name(o);
    status.message = mdbq::Hub::state_message(o);
    job.status = status;

    job.create_time = mdbq::Hub::get_create_time(o);
    if (status.state == mdbq::Hub::state_name(mdbq::TS_DONE)) {
      job.finish_time = mdbq::Hub::get_finish_time(o);
    }
    job.query_id = mdbq::Hub::get_misc(o)["query_id"].String();
    job._id = mdbq::Hub::get_id(o);

    return job;
  }

  bool Engine::request_job(const std::string& request_id, request::Job& job, std::string &msg)
  {
    mongo::BSONObj o = _hub.get_job(request_id);
    if (o.isEmpty()) {
      msg = "Request ID " + request_id + " not found.";
      return false;
    }
    job = get_job_info(o);

    return true;
  }

  bool Engine::request_jobs(const std::string &status_find, const std::string &user_key, std::vector<request::Job>& ret, std::string& msg)
  {
    utils::IdName user;
    if (!dba::users::get_user(user_key, user, msg)) {
      return false;
    }

    mdbq::TaskState task_state = mdbq::Hub::state_number(status_find);
    std::list<mongo::BSONObj> jobs_bson = _hub.get_jobs(task_state, user.id);
    for (auto &job_bson : jobs_bson) {
      ret.push_back(get_job_info(job_bson));
    }
    return true;
  }

  bool Engine::request_data(const std::string &request_id, const std::string &user_key, request::Data &data, std::string &content, request::DataType& type,  std::string &msg)
  {
    utils::IdName user;
    if (!dba::users::get_user(user_key, user, msg)) {
      return false;
    }
    mongo::BSONObj o = _hub.get_job(request_id); //TODO still check
    if (o.isEmpty()) {
      msg = "Request ID " + request_id + " not found.";
      return false;
    }

    if (!mdbq::Hub::is_done(o)) {
      msg = "Request ID " + request_id + " was not finished. Please, check its status.";
      return false;
    }

    mongo::BSONObj result = o["result"].Obj();

    if (result.hasField("__error__")) {
      msg = result["__error__"].str();
      return false;
    }

    if (result.hasField("__id_names__")) {
      mongo::BSONObj id_names = result["__id_names__"].Obj();
      data.set_id_names(utils::bson_to_id_name(id_names));
      type = request::ID_NAMES;
      return true;
    }

    if (result.hasField("__file__")) {
      std::string filename = result["__file__"].str();
      type = request::REGIONS;
      std::vector<lzo_byte> data;
      if (!_hub.get_result(filename, data, msg)) {
        return false;
      }

      const bool compressed = result.hasField("__compressed__") && result["__compressed__"].Bool();
      if (compressed) {
        long long original_size = result["__original_size__"].Long();
        size_t data_size = data.size();
        lzo_bytep data_ptr = data.data();
        size_t uncompressed_size;
        lzo_bytep decompressed_data = epidb::compress::decompress(data_ptr, data_size, original_size, uncompressed_size);

        // assert uncompressed_size == original_size

        const char* content_char = reinterpret_cast<const char *>(decompressed_data);
        content = std::string(content_char, original_size);
        free(decompressed_data);
        return true;
      } else {
        //content = sb.to_string();
      }
    }

    type = request::MAP;
    data.load_from_bson(result);

    return true;
  }

  bool Engine::user_owns_request(const std::string& request_id, const std::string& user_id)
  {
    if (_hub.job_has_user_id(request_id, user_id)) {
      return true;
    }
    return false;
  }
}