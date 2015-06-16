//
//  hub.cpp
//  epidb
//
// Code from https://github.com/temporaer/MDBQ/
// adapted to DeepBlue by Felipe Albrecht on 22.01.2015

#include <iomanip>

#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include <mongo/client/dbclient.h>

#include "hub.hpp"

#include "../log.hpp"

#include "../extras/compress.hpp"
#include "../extras/date_time.hpp"

#include "../connection/connection.hpp"

#include "../dba/helpers.hpp"
#include "../extras/utils.hpp"


#define CHECK_DB_ERR_LOG(CON)\
  {\
    std::string e = (CON)->getLastError();\
    if(!e.empty()){\
      EPIDB_LOG_ERR("hub: error_code!=0, failing: " + e + "\n" + (CON)->getLastErrorDetailed().toString());\
      return;\
    }\
  }

#define CHECK_DB_ERR_RETURN(CON, msg)\
  {\
    std::string e = (CON)->getLastError();\
    if(!e.empty()){\
      msg = "hub: error_code!=0, failing: " + e + "\n" + (CON)->getLastErrorDetailed().toString();\
      return false;\
    }\
  }

namespace mdbq {
  struct HubImpl {
    epidb::Connection m_con;

    unsigned int m_interval;
    std::string  m_prefix;
    std::auto_ptr<boost::asio::deadline_timer> m_timer;
    void print_current_job_summary(Hub *c, const boost::system::error_code &error)
    {
      std::auto_ptr<mongo::DBClientCursor> p =
        // note: currently snapshot mode may not be used w/ sorting or explicit hints
        m_con->query( m_prefix + ".jobs",
                      BSON( "state"   << mongo::GT << -1));
      CHECK_DB_ERR_LOG(m_con);

      std::cout << "JOB SUMMARY" << std::endl;
      std::cout << "===========" << std::endl
                << "Type    : "
                << std::setw(8) << "state"
                << std::setw(8) << "nfailed"
                << std::setw(10) << "owner"
                << std::setw(16) << "create_time"
                << std::setw(16) << "book_time"
                << std::setw(16) << "finish_time"
                << std::setw(16) << "deadline"
                << std::setw(16) << "misc"
                << std::endl;
      while (p->more()) {
        mongo::BSONObj f = p->next();
        if (f.isEmpty())
          continue;
        std::cout << "ASSIGNED: "
                  << std::setw(8) << f["state"].Int()
                  << std::setw(8) << f["nfailed"].Int()
                  << std::setw(10) << f["owner"].Array()[0].String()
                  << std::setw(16) << epidb::extras::dt_format(epidb::extras::to_ptime(f["create_time"].Date()))
                  << std::setw(16) << epidb::extras::dt_format(epidb::extras::to_ptime(f["book_time"].Date()))
                  << std::setw(16) << epidb::extras::dt_format(epidb::extras::to_ptime(f["finish_time"].Date()))
                  << std::setw(16) << epidb::extras::dt_format(epidb::extras::to_ptime(f["book_time"].Date()) + boost::posix_time::seconds(f["timeout"].Int()))
                  << " " << f["misc"]
                  << std::endl;
      }
    }

    void update_check(Hub *c, const boost::system::error_code &error)
    {
      // search for jobs which have failed and reschedule them
      mongo::BSONObj ret = BSON("_id" << 1 <<
                                "owner" << 1 <<
                                "nfailed" << 1);
      std::auto_ptr<mongo::DBClientCursor> p =
        m_con->query( m_prefix + ".jobs",
                      BSON(
                        "state" << TS_FAILED <<
                        "nfailed" << mongo::LT << 1), /* first time failure only */
                      0, 0, &ret);
      CHECK_DB_ERR_LOG(m_con);
      while (p->more()) {
        mongo::BSONObj f = p->next();
        if (!f.hasField("nfailed"))
          continue;

        std::cerr << "HUB: warning: task `"
                  << f["_id"] << "' on `"
                  << f["owner"].String() << "' failed, rescheduling" << std::endl;

        m_con->update(m_prefix + ".jobs",
                      BSON("_id" << f["_id"]),
                      BSON(
                        "$inc" << BSON("nfailed" << 1) <<
                        "$set" << BSON(
                          "state"         << TS_NEW
                          << "book_time"   << mongo::Undefined
                          << "refresh_time" << mongo::Undefined)));
        CHECK_DB_ERR_LOG(m_con);
      }

      if (!error) {
        m_timer->expires_at(m_timer->expires_at() + boost::posix_time::seconds(m_interval));
        m_timer->async_wait(boost::bind(&HubImpl::update_check, this, c, boost::asio::placeholders::error));
      } else {
        EPIDB_LOG_ERR("HUB: error_code!=0, failing!");
        return;
      }
    }
  };

  Hub::Hub(const std::string &url, const std::string &prefix)
    : m_prefix(prefix)
  {
    m_ptr.reset(new HubImpl());
    m_ptr->m_con->createCollection(prefix + ".jobs");
    m_ptr->m_prefix = prefix;
  }

  bool Hub::insert_job(const mongo::BSONObj &job, unsigned int timeout, const int version_value, std::string &id, std::string &msg)
  {
    int r_id;
    if (!epidb::dba::helpers::get_increment_counter("request", r_id, msg))  {
      return false;
    }

    id = "r" + epidb::utils::integer_to_string(r_id);

    boost::posix_time::ptime ctime = epidb::extras::universal_date_time();
    m_ptr->m_con->insert(m_prefix + ".jobs",
                         BSON( "_id" << id
                               << "timeout"     << timeout
                               << "version"     << version_value
                               << "create_time" << epidb::extras::to_mongo_date(ctime)
                               << "finish_time" << mongo::Undefined
                               << "book_time"   << mongo::Undefined
                               << "refresh_time" << mongo::Undefined
                               << "misc"        << job
                               << "nfailed"     << (int)0
                               << "state"       << TS_NEW
                               << "version"     << (int)0
                             )
                        );
    CHECK_DB_ERR_RETURN(m_ptr->m_con, msg);

    return true;
  }

  size_t Hub::get_n_open()
  {
    return m_ptr->m_con->count(m_prefix + ".jobs",
                               BSON( "state" << TS_NEW));
  }

  size_t Hub::get_n_assigned()
  {
    return m_ptr->m_con->count(m_prefix + ".jobs",
                               BSON( "state" << TS_RUNNING));
  }

  size_t Hub::get_n_ok()
  {
    return m_ptr->m_con->count(m_prefix + ".jobs",
                               BSON( "state" << TS_DONE));
  }

  size_t Hub::get_n_failed()
  {
    return m_ptr->m_con->count(m_prefix + ".jobs",
                               BSON( "state" << TS_FAILED));
  }

  void Hub::clear_all()
  {
    m_ptr->m_con->dropCollection(m_prefix + ".jobs");
    m_ptr->m_con->dropCollection(m_prefix + ".log");
    m_ptr->m_con->dropCollection(m_prefix + ".fs.chunks");
    m_ptr->m_con->dropCollection(m_prefix + ".fs.files");

    // this is from https://jira.mongodb.org/browse/SERVER-5323
    // m_ptr->m_con->createIndex(m_prefix + ".fs.chunks", BSON("files_id" << 1 << "n" << 1));
  }

  void Hub::got_new_results()
  {
    std::cout << "New results available!" << std::endl;
  }

  void Hub::reg(boost::asio::io_service &io_service, unsigned int interval)
  {
    m_ptr->m_interval = interval;
    m_ptr->m_timer.reset(new boost::asio::deadline_timer(io_service, boost::posix_time::seconds(interval)));
    m_ptr->m_timer->async_wait(boost::bind(&HubImpl::update_check, m_ptr.get(), this, boost::asio::placeholders::error));
  }

  mongo::BSONObj Hub::get_job(const std::string &id)
  {
    return m_ptr->m_con->findOne(m_prefix + ".jobs", BSON("_id" << id));
  }

  std::list<mongo::BSONObj> Hub::get_jobs(const mdbq::TaskState& state, const std::string &user_id)
  {
    std::auto_ptr<mongo::DBClientCursor> cursor;
    if (state == mdbq::_TS_END) {
      cursor = m_ptr->m_con->query(m_prefix + ".jobs", BSON("misc.user_id" << user_id));
    } else {
      cursor = m_ptr->m_con->query(m_prefix + ".jobs", BSON("state" << state << "misc.user_id" << user_id));
    }

    std::list<mongo::BSONObj> ret;
    while (cursor->more()) {
      mongo::BSONObj o = cursor->next().getOwned();
      ret.push_back(o);
    }
    return ret;
  }

  bool Hub::job_has_user_id(const std::string& request_id, const std::string& user_id)
  {
    return m_ptr->m_con->count(m_prefix + ".jobs", BSON("_id" << request_id << "misc.user_id" << user_id)) > 0;
  }

  mongo::BSONObj Hub::get_newest_finished()
  {
    return m_ptr->m_con->findOne(m_prefix + ".jobs",
                                 mongo::Query(BSON("state" << TS_DONE)).sort("finish_time"));
  }

  mdbq::TaskState Hub::state_number(const std::string& name)
  {
    if (name == "new") {
      return TS_NEW;
    } else if (name == "running") {
      return TS_RUNNING;
    } else if (name == "done") {
      return TS_DONE;
    } else if (name == "failed") {
      return TS_FAILED;
    } else {
      return _TS_END;
    }
  }

  std::string Hub::state_name(const mongo::BSONObj &o)
  {
    return state_name(o["state"].Int());
  }

  std::string Hub::state_name(const int state)
  {
    switch (state) {
    case TS_NEW:
      return "new";
    case TS_RUNNING:
      return "running";
    case TS_DONE:
      return "done";
    case TS_FAILED:
      return "failed";
    default :
      return "Invalid State: " + epidb::utils::integer_to_string(state);
    }
  }

  std::string Hub::state_message(const mongo::BSONObj &o)
  {
    int state = o["state"].Int();

    switch (state) {
    case TS_NEW:
      return "";
    case TS_RUNNING:
      return "";
    case TS_DONE:
      return "";
    case TS_FAILED:
      return "**failed message**"; // TODO: implement here
    default :
      return "Invalid State: " + epidb::utils::integer_to_string(state);
    }
  }

  boost::posix_time::ptime Hub::get_create_time(const mongo::BSONObj& o)
  {
    return epidb::extras::to_ptime(o["create_time"].Date());
  }

  boost::posix_time::ptime Hub::get_finish_time(const mongo::BSONObj& o)
  {
    return epidb::extras::to_ptime(o["finish_time"].Date());
  }

  mongo::BSONObj Hub::get_misc(const mongo::BSONObj& o)
  {
    return o["misc"].Obj();
  }

  std::string Hub::get_id(const mongo::BSONObj& o)
  {
    return o["_id"].String();
  }

  bool Hub::is_done(const mongo::BSONObj &o)
  {
    return o["state"].Int() == TS_DONE;
  }

  bool Hub::get_file_info(const std::string &filename, mongo::OID &oid, size_t &chunk_size, size_t &file_size, std::string &msg)
  {
    std::auto_ptr<mongo::DBClientCursor> data_cursor = m_ptr->m_con->query(m_ptr->m_prefix + ".fs.files", mongo::Query(BSON("filename" << filename)));

    if (data_cursor->more()) {
      auto fileinfo = data_cursor->next();
      oid = fileinfo["_id"].OID();
      chunk_size = fileinfo["chunkSize"].numberLong();
      file_size = fileinfo["length"].numberLong();
      return true;
    }

    msg = "The result data under the request '" + filename + "'' was not found";
    return false;
  }

  bool Hub::get_result(const std::string &filename, std::string &content, std::string &msg)
  {
    mongo::OID oid;
    size_t file_size;
    size_t chunk_size;
    if (!get_file_info(filename, oid, chunk_size, file_size, msg)) {
      return false;
    }

    mongo::BSONObj projection = BSON("data" << 1);

    size_t remaining = file_size;
    size_t n = 0;

    std::stringstream ss;

    while (remaining > 0) {
      mongo::Query q(BSON("files_id" << oid << "n" << (long long) n));
      std::auto_ptr<mongo::DBClientCursor> data_cursor = m_ptr->m_con->query(m_ptr->m_prefix + ".fs.chunks", q, 0, 0, &projection);
      if (data_cursor->more()) {
        int read;
        char* compressed_data = (char *) data_cursor->next().getField("data").binData(read);
        ss.write(compressed_data, read);

        n++;
        remaining -= read;
      } else {
        msg = "Chunk for file " + filename + " not found.";
        return false;
      }
    }
    content = ss.str();
    return true;
  }

}
