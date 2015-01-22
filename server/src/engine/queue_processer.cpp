//
//  queue+processer.cpp
//  epidb
//
//  Created by Felipe Albrecht on 22.01.15.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include <boost/asio.hpp>

#include <mongo/client/dbclient.h>

#include "../dba/config.hpp"

#include "../mdbq/client.hpp"

namespace epidb {
  class QueueHandler : public mdbq::Client {
    public:
    	QueueHandler(std::string &url, std::string &prefix) :
      mdbq::Client(url, prefix) {}


    void handle_task(const mongo::BSONObj &o)
    {
      try {
        std::cerr << o.toString() << std::endl;
      } catch (mdbq::timeout_exception) {
        /* do nothing */
      }
    }
  };


  void processer_run(size_t num, std::string url, std::string prefix)
  {
    boost::asio::io_service io;

    std::string server = dba::config::get_mongodb_server();
    std::string collection = dba::config::DATABASE_NAME() + "_queue";
    for (size_t i = 0; i < num; i++) {
      QueueHandler qh(server, collection);
      qh.reg(io, 0.5);
    }
    io.run();
  }

}