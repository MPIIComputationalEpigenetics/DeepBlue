//
//  queue_processer.cpp
//  epidb
//
//  Created by Felipe Albrecht on 23.01.15.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include <boost/asio.hpp>

#include "../mdbq/client.hpp"

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
      void handle_task(const mongo::BSONObj &o);
      void run();

      mongo::BSONObj process(const mongo::BSONObj &job);
      mongo::BSONObj process_count(const std::string &request_id, const std::string &user_key);
      mongo::BSONObj process_get_regions(const std::string &query_id, const std::string &format, const std::string &user_key);
    };
  }
}