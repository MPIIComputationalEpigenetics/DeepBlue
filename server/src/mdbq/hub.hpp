//
//  hub.hpp
//  epidb
//
//  Created by Felipe Albrecht on 22.01.15.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//
// From: https://github.com/temporaer/MDBQ/

#ifndef __MDBQ_HUB_HPP__
#     define __MDBQ_HUB_HPP__

#include <boost/shared_ptr.hpp>

namespace mongo {
  class  BSONObj;
}
namespace boost {
  namespace asio {
    class io_service;
  }
}
namespace mdbq {
  struct HubImpl;

  /**
   * MongoDB Queue Hub
   *
   * derive from this class to implement your own job generator
   */
  class Hub {
  private:
    /// pointer to implementation
    boost::shared_ptr<HubImpl> m_ptr;

    /// database plus queue prefix (db.queue)
    const std::string m_prefix;
  public:
    /**
     * ctor.
     *
     * @param url how to connect to mongodb
     */
    Hub(const std::string &url, const std::string &prefix);

    /**
     * insert job
     *
     * @param job the job description
     * @param timeout the timeout in seconds
     * @param version the DeepBlue version when the job was inserted
     */
    bool insert_job(const mongo::BSONObj &job, unsigned int timeout, const int version_value, std::string &id, std::string &msg);


    mongo::BSONObj get_job(const std::string& id, const std::string& user_key);


    /**
     * get newest finished job (primarily for testing)
     */
    mongo::BSONObj get_newest_finished();


    /**
     * get number of pending jobs
     */
    size_t get_n_open();

    /**
     * get number of jobs being worked on
     */
    size_t get_n_assigned();

    /**
     * get number of jobs finished
     */
    size_t get_n_ok();

    /**
     * get number of jobs failed
     */
    size_t get_n_failed();

    /**
     * clear the whole job queue
     */
    void clear_all();

    /**
     * register with the main loop
     *
     * @param interval querying interval
     */
    void reg(boost::asio::io_service &io_service, unsigned int interval);

    virtual void got_new_results();

    static std::string state_name(mongo::BSONObj& o);

    static std::string state_message(mongo::BSONObj& o);

    static bool is_done(mongo::BSONObj& o);
  };
}
#endif /* __MDBQ_HUB_HPP__ */
