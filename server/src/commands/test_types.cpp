//
//  dummy.cpp
//  DeepBlue Epigenomic Data Server

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
//  Created by Felipe Albrecht on 30.07.13.
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

#include "../extras/serialize.hpp"
#include "../dba/dba.hpp"

#include "../engine/commands.hpp"

namespace epidb {
  namespace command {

    class TestTypesCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::ADMINISTRATION, "Debugging command to test type serialization.");
      }

      static Parameters parameters_()
      {
        Parameter p[] = {
          Parameter("string", serialize::STRING, ""),
          Parameter("int", serialize::INTEGER, ""),
          Parameter("double", serialize::DOUBLE, ""),
          Parameter("bool", serialize::BOOLEAN, ""),
          Parameter("list", serialize::LIST, ""),
          Parameter("map", serialize::MAP, "")
        };
        Parameters params(&p[0], &p[0] + 6);
        return params;
      }

      static Parameters results_()
      {
        Parameter p[] = {
          Parameter("string", serialize::STRING, ""),
          Parameter("int", serialize::INTEGER, ""),
          Parameter("double", serialize::DOUBLE, ""),
          Parameter("boolean", serialize::BOOLEAN, ""),
          Parameter("list", serialize::LIST, ""),
          Parameter("map", serialize::MAP, "")
        };
        Parameters results(&p[0], &p[0] + 6);
        return results;
      }

    public:
      TestTypesCommand() : Command("test_types", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        const std::string string_ = parameters[0]->as_string();
        const unsigned int integer_ = parameters[1]->as_number();
        const double double_ = parameters[2]->as_number();
        bool bool_ = parameters[3]->as_boolean();

        result.add_string(string_);
        result.add_int(integer_);
        result.add_double(double_);
        result.add_bool(bool_);

        // std::vector<serialize::ParameterPtr> list_;
        // if (!parameters[4]->children(list_)) {
        //   result.add_error("unable to read list");
        //   return false;
        // }
        // std::vector<std::string> list_ret;
        // std::vector<serialize::ParameterPtr>::iterator vit;
        // for (vit = list_.begin(); vit != list_.end(); ++vit) {
        //   list_ret.push_back(*(vit)->as_string());
        // }
        // result.add_list(list_ret);

        // std::map<std::string, serialize::ParameterPtr> map_;
        // if (!parameters[5]->children(map_)) {
        //   result.add_error("unable to read map");
        //   return false;
        // }

        // std::map<std::string, std::string> map_ret;
        // std::map<std::string, serialize::ParameterPtr>::iterator  mit;
        // for (mit = map_.begin(); mit != map_.end(); ++mit) {
        //   list_ret[mit->first] = mit->second->as_string();
        // }
        // result.add_map(map_ret);

        result.add_param(parameters[4]);
        result.add_param(parameters[5]);

        return true;
      }

    } testTypesCommand;
  }
}
