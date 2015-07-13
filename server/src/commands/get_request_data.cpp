//
//  get_request_data.cpp
//  epidb
//
//  Created by Felipe Albrecht on 27.01.15.
//  Copyright (c) 2013,2014,2015 Max Planck Institute for Computer Science. All rights reserved.
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
#include "../entities/users.hpp"
#include "../engine/commands.hpp"
#include "../engine/engine.hpp"
#include "../engine/request.hpp"

#include "../errors.hpp"

#include "../extras/serialize.hpp"
#include "../extras/utils.hpp"

namespace epidb {
  namespace command {

    class GetRequestDataCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::REQUESTS, "Get the status of the given request.");
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
          Parameter("information", serialize::MAP, "Maps containing the request data", true)
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
        
        datatypes::User user1;
        if (!dba::get_user_by_key(user_key, user1, msg)) {
          result.add_error(msg);
          return false;
        }
        
        if (!user1.has_permission(datatypes::GET_DATA)) {
            result.add_error(Error::m(ERR_INSUFFICIENT_PERMISSION));
            return false;
        }

        utils::IdName user;
        if (!dba::users::get_user(user_key, user, msg)) {
          return false;
        }

        std::string file_content;
        request::Data data;
        request::DataType type = request::DataType::INVALID;
        if (!epidb::Engine::instance().user_owns_request(request_id, user.id)) {
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
