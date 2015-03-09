//
//  engine.hpp
//  epidb
//
//  Created by Felipe Albrecht on 29.05.13.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#ifndef EPIDB_ENGINE_ENGINE_HPP
#define EPIDB_ENGINE_ENGINE_HPP

#include <boost/noncopyable.hpp>

#include "commands.hpp"
#include "request.hpp"

#include "../mdbq/hub.hpp"

#include "../log.hpp"

namespace epidb {
  class Engine : private boost::noncopyable {
  private:
    mdbq::Hub _hub;

    Engine();
    Engine(Engine const &);
    void operator=(Engine const &);

    bool queue(const mongo::BSONObj &job, unsigned int timeout, std::string &request_id, std::string &msg);

  public:
    static Engine &instance()
    {
      static Engine instance;
      return instance;
    }

    bool init();

    bool execute(const std::string &name, const std::string &ip, unsigned long long id,
                 serialize::Parameters &parameters, serialize::Parameters &result) const;

    bool request_status(const std::string &request_id, const std::string &user_key, request::Status &status, std::string &msg);

    bool request_data(const std::string &request_id, const std::string &user_key, request::Data &data, StringBuilder &sb, request::DataType &type, std::string &msg);

    bool queue_count_regions(const std::string &query_id, const std::string &user_key, std::string &request_id, std::string &msg);

    bool queue_get_regions(const std::string &query_id, const std::string &output_format, const std::string &user_key, std::string &id, std::string &msg);

    bool queue_score_matrix(const std::vector<std::pair<std::string, std::string>> &experiments_formats, const std::string &aggregation_function, const std::string &regions_query_id, const std::string &user_key, std::string &request_id, std::string &msg);

    bool queue_get_experiments_by_query(const std::string &query_id, const std::string &user_key, std::string &request_id, std::string &msg);
  };
}

#endif
