//
//  get_request_data.cpp
//  epidb
//
//  Created by Felipe Albrecht on 27.01.15.
//  Copyright (c) 2013,2014,2015 Max Planck Institute for Computer Science. All rights reserved.
//

#include <sstream>
#include <map>

#include "../dba/dba.hpp"
#include "../dba/queries.hpp"
#include "../dba/users.hpp"

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
        const std::string query_id = parameters[0]->as_string();
        const std::string user_key = parameters[1]->as_string();

        std::string msg;
        if (!Command::checks(user_key, msg)) {
          result.add_error(msg);
          return false;
        }

        utils::IdName user;
        if (!dba::users::get_user(user_key, user, msg)) {
          return false;
        }

        bool admin_key = false;
        if (! dba::users::is_admin_key(user_key, admin_key, msg)) {
          return false;
        }
        
        StringBuilder sb;
        request::Data data;
        request::DataType type = request::DataType::INVALID;
        if (admin_key || epidb::Engine::instance().user_owns_request(query_id, user.id)) {
          if (!epidb::Engine::instance().request_data(query_id, user_key, data, sb, type, msg)) {
            result.add_error(msg);
            return false;
          }
        } else {
          result.add_error("Request ID " + query_id + " not found.");
          return false;
        }

        if (type == request::ID_NAMES) {
          set_id_names_return(data.id_names, result);
          return true;
        }

        if (type == request::REGIONS) {
          result.add_stringbuilder(sb);
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
