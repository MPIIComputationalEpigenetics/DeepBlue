//
//  hub.hpp
//  epidb
//
// Code from https://github.com/temporaer/MDBQ/
// adapted to DeepBlue by Felipe Albrecht on 22.01.2015

#ifndef __MDBQ_HUB_HPP__
#     define __MDBQ_HUB_HPP__

#include <boost/shared_ptr.hpp>
#include "../mdbq/common.hpp"

#include "../extras/compress.hpp"

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

namespace mongo {
  class  BSONObj;
  class DBClientCursor;
}
namespace boost {
  namespace asio {
    class io_service;
  }
}

namespace epidb {
  class StringBuilder;
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

    bool get_file_info(const std::string &filename, mongo::OID& oid, size_t &chunk_size, size_t &file_size, std::string &msg);

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


    mongo::BSONObj get_job(const std::string& id);

    /*
     * \brief Get all jobs in given state owned by given user. If no valid mdbq::TaskState is given,
     *        all jobs are returned.
     * \param state     State searched for
     *        user_id   ID of owning user
     * \return          Requested jobs
     */
    std::list<mongo::BSONObj> get_jobs(const mdbq::TaskState& state, const std::string &user_id);

    bool job_has_user_id(const std::string& request_id, const std::string& user_id);

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

    bool get_result(const std::string &filename, std::string& data, std::string &msg);

    /**
     * register with the main loop
     *
     * @param interval querying interval
     */
    void reg(boost::asio::io_service &io_service, unsigned int interval);

    virtual void got_new_results();

    static mdbq::TaskState state_number(const std::string& name);

    static std::string state_name(const mongo::BSONObj& o);

    static std::string state_name(const int state);

    static std::string state_message(const mongo::BSONObj& o);

    /*
    * \brief Get the time at which a request was created
    */
    static boost::posix_time::ptime get_create_time(const mongo::BSONObj& o);

    /*
    * \brief Get the time at which a request was completed
    */
    static boost::posix_time::ptime get_finish_time(const mongo::BSONObj& o);

    /*
    * \brief Get misc info for a request
    */
    static mongo::BSONObj get_misc(const mongo::BSONObj& o);

    /*
    * \brief Get the id of a request
    */
    static std::string get_id(const mongo::BSONObj& o);

    static bool is_done(const mongo::BSONObj& o);
  };
}
#endif /* __MDBQ_HUB_HPP__ */
