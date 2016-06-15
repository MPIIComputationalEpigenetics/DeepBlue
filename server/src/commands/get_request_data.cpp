//
//  get_request_data.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 27.01.15.
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
#include <sstream>
#include <map>

#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/bzip2.hpp>
#include <boost/iostreams/stream.hpp>

#include "../dba/dba.hpp"
#include "../dba/queries.hpp"
#include "../dba/users.hpp"

#include "../datatypes/user.hpp"

#include "../engine/commands.hpp"
#include "../engine/engine.hpp"
#include "../engine/request.hpp"

#include "../extras/serialize.hpp"
#include "../extras/utils.hpp"

#include "../errors.hpp"

namespace epidb {
  namespace command {

    class GetRequestDataCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::REQUESTS, "Download the request data. The output can be (i) a string (get_regions, score_matrix, and count_regions), or (ii) a list of ID and names (get_experiments_by_query).");
      }

      static  Parameters parameters_()
      {
        Parameter p[] = {
          Parameter("request_id", serialize::STRING, "ID of the request"),
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 2);
        return params;
      }

      static Parameters results_()
      {
        Parameter p[] = {
          Parameter("data", serialize::STRING, "the request data", true)
        };
        Parameters results(&p[0], &p[0] + 1);
        return results;
      }

    public:
      GetRequestDataCommand() : Command("get_request_data", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        const std::string request_id = parameters[0]->as_string();
        const std::string user_key = parameters[1]->as_string();

        std::string msg;
        datatypes::User user;

        if (!check_permissions(user_key, datatypes::GET_DATA, user, msg )) {
          result.add_error(msg);
          return false;
        }

        std::string file_content;
        request::Data data;
        request::DataType type = request::DataType::INVALID;
        if (!epidb::Engine::instance().user_owns_request(request_id, user.get_id())) {
          result.add_error("Request ID " + request_id + " not found.");
          return false;
        }

        if (!epidb::Engine::instance().request_data(request_id, user_key, data, file_content, type, msg)) {
          result.add_error(msg);
          return false;
        }

        if (type == request::ID_NAMES) {
          set_id_names_return(data.id_names, result);
          return true;
        }

        if (type == request::REGIONS) {
          std::istringstream inStream(file_content, std::ios::binary);
          std::stringstream outStream;
          boost::iostreams::filtering_streambuf< boost::iostreams::input> in;
          in.push( boost::iostreams::bzip2_decompressor());
          in.push( inStream );
          boost::iostreams::copy(in, outStream);
          result.add_string_content(outStream.str());
          return true;
        }

        if (type == request::MAP) {
          serialize::ParameterPtr map(new serialize::MapParameter());
          for (auto &ss : data.strings) {
            serialize::ParameterPtr p(new serialize::SimpleParameter(ss.second));
            map->add_child(ss.first, std::move(p));
          }

          for (auto is : data.integers) {
            serialize::ParameterPtr p(new serialize::SimpleParameter(is.second));
            map->add_child(is.first, p);
          }

          for (auto fs : data.floats) {
            serialize::ParameterPtr p(new serialize::SimpleParameter(fs.second));
            map->add_child(fs.first, p);
          }
          result.add_param(map);
          return true;
        }

        result.add_error("Internal Error: Invalid data type.");
        return false;
      }
    } getRequestDataCommand;
  }
}
