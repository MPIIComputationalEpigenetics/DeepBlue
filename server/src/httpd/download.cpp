//
//  download.cpp
//  epidb
//
//  Created by Felipe Albrecht on 16.06.15.
//  Copyright (c) 2015 Max Planck Institute for Computer Science. All rights reserved.
//

#include <iostream>
#include <regex>
#include <string>
#include <sstream>

#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/bzip2.hpp>
#include <boost/iostreams/stream.hpp>

#include "../dba/users.hpp"
#include "../engine/engine.hpp"
#include "../engine/request.hpp"
#include "reply.hpp"

namespace epidb {
  namespace httpd {

    Reply get_download_data(const std::string& uri)
    {
      std::smatch sm;
      if (!std::regex_match(uri, sm, std::regex("/download\\\?r=(\\w+)&key=(\\w+)"))) {
        return Reply::stock_reply(Reply::bad_request, "Invalid request: " + uri);
      }

      std::string request_id = sm[1];
      std::string user_key = sm[2];

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
