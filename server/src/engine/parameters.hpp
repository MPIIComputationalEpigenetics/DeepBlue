//
//  commands.hpp
//  epidb
//
//  Created by Felipe Albrecht on 29.05.13.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#ifndef EPIDB_ENGINE_PARAMETERS_HPP
#define EPIDB_ENGINE_PARAMETERS_HPP

#include <iostream>
#include <map>
#include <sstream>
#include <stdlib.h>
#include <string>
#include <vector>

#include "../extras/xmlrpc.hpp"
#include "../extras/serialize.hpp"

namespace epidb {
  class Parameter {
  private:
    std::string name_;
    serialize::Type type_;
    std::string description_;
    bool multiple_;

  public:
    Parameter(std::string name, serialize::Type
              type, std::string desc, bool multiple = false)
      : name_(name), type_(type), description_(desc), multiple_(multiple) {}

    const std::string name() const
    {
      return name_;
    }

    const std::string description() const
    {
      return description_;
    }

    bool multiple() const
    {
      return multiple_;
    }

    serialize::Type type() const
    {
      return type_;
    }
  };

  typedef std::vector<Parameter> Parameters;

  namespace parameters {
    const Parameter Genome("genome", serialize::STRING, "the target genome");
    const Parameter GenomeMultiple("genome", serialize::STRING, "the target genome", true);
    const Parameter UserKey("user_key", serialize::STRING, "users token key");
  }
} // namespace epidb

#endif
