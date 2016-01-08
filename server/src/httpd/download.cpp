//
//  download.cpp
//  epidb
//
//  Created by Felipe Albrecht on 16.06.15.
//  Copyright (c) 2016 Max Planck Institute for Informatics. All rights reserved.
//

#include <iostream>
#include <regex>
#include <string>
#include <sstream>

#include <boost/algorithm/string.hpp>

#include "../dba/users.hpp"
#include "../engine/engine.hpp"
#include "../engine/request.hpp"
#include "reply.hpp"

namespace epidb {
  namespace httpd {

    Reply get_download_data(const std::string& uri)
    {
      std::vector<std::string> strs;
      boost::split(strs, uri, boost::is_any_of("?"));

      if (strs.size() != 2) {
        return Reply::stock_reply(Reply::bad_request, "Invalid request, it must be: /download?r_id=REQUEST_ID&key=USER_KEY");
      }

      std::vector<std::string> params;
      boost::split(params, strs[1], boost::is_any_of("&"));
      if (params.size() != 2) {
        return Reply::stock_reply(Reply::bad_request, "Invalid request, it was not possible to read the parameters. It must be: /download?r_id=REQUEST_ID&key=USER_KEY");
      }

      std::vector<std::string> request;
      boost::split(request, params[0], boost::is_any_of("="));
      if (request.size() != 2) {
        return Reply::stock_reply(Reply::bad_request, "Invalid request, it was not possible to read the request ID. It must be: /download?r_id=REQUEST_ID&key=USER_KEY");
      }
      const std::string request_id = request[1];


      std::vector<std::string> key;
      boost::split(key, params[1], boost::is_any_of("="));
      if (key.size() != 2) {
        return Reply::stock_reply(Reply::bad_request, "Invalid request, it was not possible to read the user_key.  It must be: /download?r_id=REQUEST_ID&key=USER_KEY");
      }
      const std::string user_key = key[1];

      std::string msg;
      utils::IdName user;
      if (!dba::users::get_user(user_key, user, msg)) {
        return Reply::stock_reply(Reply::bad_request, "Invalid request: " + msg);
      }

      std::string content;
      request::Data data;
      request::DataType type = request::DataType::INVALID;
      if (!epidb::Engine::instance().user_owns_request(request_id, user.id)) {
        return Reply::stock_reply(Reply::bad_request, "Invalid request: User " + user.name + "/" + user.id + " does not have the request " + request_id);
      }

      if (!epidb::Engine::instance().request_data(request_id, user_key, data, content, type, msg)) {
        return Reply::stock_reply(Reply::bad_request, "Invalid request: " + msg);
      }

      if (type == request::REGIONS) {
        return Reply::stock_reply_download(Reply::ok, request_id, std::move(content));
      }

      return Reply::stock_reply(Reply::bad_request, "Invalid data");
    }

  } // namespace httpd
} // namespace epidb
