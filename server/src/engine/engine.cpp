//
//  engine.cpp
//  epidb
//
//  Created by Felipe Albrecht on 08.04.14.
//  Copyright (c) 2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include <iostream>
#include <sstream>
#include <vector>

#include "commands.hpp"
#include "engine.hpp"
#include "../log.hpp"

#include "../dba/config.hpp"
#include "../dba/queries.hpp"

#include "../version.hpp"

namespace epidb {

  Engine::Engine()
    : _hub(dba::config::get_mongodb_server(), dba::config::DATABASE_NAME() + "_queue")
  {
    EPIDB_LOG("Creating Engine");
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
    if (!queue(BSON("command" << "count_regions" << "query_id" << query_id << "user_key" << user_key), 60 * 60, id, msg)) {
      return false;
    }
    return true;
  }


  mongo::BSONObj Engine::process(const mongo::BSONObj &job)
  {
    std::cerr << job.toString() << std::endl;
    std::string command = job["command"].str();

    if (command == "count_regions") {
      return process_count(job["query_id"].str(), job["user_key"].str());
    } else {
      mongo::BSONObjBuilder bob;
      bob.append("success", false);
      bob.append("error", "Invalid command" + command);
      return bob.obj();
    }
  }

  mongo::BSONObj Engine::process_count(const std::string &query_id, const std::string &user_key)
  {
    std::string msg;
    mongo::BSONObjBuilder bob;
    size_t count = 0;

    if (!dba::query::count_regions(query_id, user_key, count, msg)) {
      bob.append("error", msg);
      return bob.obj();
    }

    bob.append("count", (long long) count);

    return bob.obj();
  }

  bool Engine::request_status(const std::string &request_id, const std::string &user_key, request::Status &request_status, std::string &msg)
  {
    mongo::BSONObj o = _hub.get_job(request_id, user_key);
    if (o.isEmpty()) {
      msg = "Request ID " + request_id + " not found.";
      return false;
    }

    request_status.state = mdbq::Hub::state_name(o);
    request_status.message = mdbq::Hub::state_message(o);
    return true;
  }

  bool Engine::request_data(const std::string &request_id, const std::string &user_key, request::Data &data, std::string &msg)
  {
    mongo::BSONObj o = _hub.get_job(request_id, user_key);
    if (o.isEmpty()) {
      msg = "Request ID " + request_id + " not found.";
      return false;
    }

    if (!mdbq::Hub::is_done(o)) {
      msg = "Request ID " + request_id + " was not finished. Please, check its status.";
      return false;
    }

    mongo::BSONObj result = o["result"].Obj();

    data.load_from_bson(result);

    return true;
  }
}
