//
//  commands.hpp
//  epidb
//
//  Created by Felipe Albrecht on 29.05.13.
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
