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

  bool Engine::queue(const mongo::BSONObj &job, unsigned int timeout, std::string& id, std::string &msg)
  {
    if (!_hub.insert_job(job, timeout, Version::version(), id, msg)) {
      return false;
    }
    return true;
  }

  bool Engine::queue_count_regions(const std::string &query_id, const std::string &user_key, std::string& id, std::string &msg)
  {
    if (!queue(BSON("command" << "count_regions" << "query_id" << query_id << "user_key" << user_key), 60 * 60, id, msg)) {
      return false;
    }
    return true;
  }
}
