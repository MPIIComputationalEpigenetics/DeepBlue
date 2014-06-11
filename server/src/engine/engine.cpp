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


namespace epidb {
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
                parameters.string() << "). " << msg);
      result.add_error(msg);
      return false;
    }

    EPIDB_LOG("Request (" << id << ") from " << ip << ": " << name << " with (" <<  parameters.string() << ").");
    return command->run(ip, parameters, result);
  }
}
