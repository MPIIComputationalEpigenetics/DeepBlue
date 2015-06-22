//
//  queue_processer.cpp
//  epidb
//
//  Created by Felipe Albrecht on 23.01.15.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include <boost/asio.hpp>

#include "../mdbq/client.hpp"
#include "../processing/processing.hpp"

namespace mongo {
  class BSONObj;
}

namespace epidb {
  namespace engine {
    void queue_processer_run(size_t num);
    class QueueHandler : public mdbq::Client {
    public:
      boost::asio::io_service ios;
      size_t _id;
      QueueHandler(size_t id, std::string &url, std::string &prefix);
      void handle_task(const std::string& _id, const mongo::BSONObj &o);
      void run();

      bool process(const mongo::BSONObj &job, processing::StatusPtr status, mongo::BSONObj& result);
      bool process_count(const std::string &request_id, const std::string &user_key, processing::StatusPtr status, mongo::BSONObj& result);
      bool process_get_regions(const std::string &query_id, const std::string &format, const std::string &user_key, processing::StatusPtr status, mongo::BSONObj& result);
      bool process_score_matrix(const mongo::BSONObj &experiments_formats, const std::string &aggregation_function, const std::string &regions_query_id, const std::string &user_key, processing::StatusPtr status, mongo::BSONObj& result);
      bool process_get_experiments_by_query(const std::string &query_id, const std::string &user_key, processing::StatusPtr status, mongo::BSONObj& result);
    };
  }
}