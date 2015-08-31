//
//  client.hpp
//  epidb
//
// Code from https://github.com/temporaer/MDBQ/
// adapted to DeepBlue by Felipe Albrecht on 22.01.2015

#ifndef __MDBQ_CLIENT_HPP__
#     define __MDBQ_CLIENT_HPP__

#include <stdexcept>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <string>

namespace mongo {
  class BSONObj;
}
namespace boost {
  namespace asio {
    class io_service;
  }
}

namespace epidb {
  namespace mdbq {

    class timeout_exception : public std::runtime_error {
    public:
      timeout_exception() : std::runtime_error("MDBQ Timeout") {}
    };


    struct ClientImpl;
    class Client {
    private:
      boost::shared_ptr<ClientImpl> m_ptr;
      std::string m_jobcol;
      std::string m_fscol;
      std::string m_db;
      bool m_verbose;

    public:
      /**
       * construct client w/o task preferences.
       *
       * @param url the URL of the mongodb server
       * @param prefix the name of the database
       */
      Client(const std::string &url, const std::string &prefix);
      /**
       * construct client with task preferences.
       *
       * @param url the URL of the mongodb server
       * @param prefix the name of the database
       * @param q query selecting certain types of tasks
       */
      Client(const std::string &url, const std::string &prefix, const mongo::BSONObj &q);

      /**
       * acquire a new task in o.
       */
      bool get_next_task(std::string &_id, mongo::BSONObj &o);

      /**
       * finish the task.
       * @param result a description of the result
       * @param ok if false, task may be rescheduled by hub
       */
      void finish(const mongo::BSONObj &result, bool ok = 1);

      /**
       * register with the main loop
       *
       * @param interval querying interval in seconds
       */
      void reg(boost::asio::io_service &io_service, float interval);

      /**
       * get the log of a task (mainly for testing)
       */
      std::vector<mongo::BSONObj> get_log(const mongo::BSONObj &task);

      /**
       * This function should be overwritten in real clients.
       *
       * @param task the task _id and description.
       */
      virtual void handle_task(const std::string& _id, const mongo::BSONObj &task);


      /**
       * Store the result data into GSF
       *
       * @param ptr pointer to the data
       * @param len data size
       */
      std::string store_result(const char *ptr, size_t len);

      /**
       * Destroy client.
       */
      virtual ~Client();

      /**
       * set client verbosity.
       * @param v verbosity
       */
      inline void set_verbose(bool v = true)
      {
        m_verbose = v;
      }
    };
  }
}
#endif /* __MDBQ_CLIENT_HPP__ */
