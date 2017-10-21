//
//  hub.hpp
//  DeepBlue Epigenomic Data Server

//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.

//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.

//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// Code from https://github.com/temporaer/MDBQ/
// adapted to DeepBlue by Felipe Albrecht on 22.01.2015

#ifndef __MDBQ_HUB_HPP__
#define __MDBQ_HUB_HPP__

#include <memory>
#include "../datatypes/user.hpp"
#include "../mdbq/common.hpp"

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

namespace epidb {
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
      std::shared_ptr<HubImpl> m_ptr;

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
       * Check if job with the given parameters already exists.
       */
      bool exists_job(const mongo::BSONObj &job, std::string &id, bool update = false);

      /**
       * insert job
       *
       * @param job the job description
       * @param timeout the timeout in seconds
       * @param version the DeepBlue version when the job was inserted
       */
      bool insert_job(const mongo::BSONObj &job, unsigned int timeout, const int version_value, std::string &id, std::string &msg);


      mongo::BSONObj get_job(const std::string& id, bool update_access = false);

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
       * Cancel, stop, or delete a request
       */
      bool cancel_request(const datatypes::User &user, const std::string& request_id, std::string& msg);


      /**
       *
       */
      bool remove_request_data(const std::string& request_id, TaskState state, std::string& msg);


      /**
       *
       */
      bool reprocess_job(const mongo::BSONObj &job);

      /**
       * register with the main loop
       *
       * @param interval querying interval
       */
      void reg(boost::asio::io_service &io_service, unsigned int interval);

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

      static bool is_cleared(const mongo::BSONObj& o);

      static bool is_failed(const mongo::BSONObj& o);
    };
  }
}
#endif /* __MDBQ_HUB_HPP__ */
