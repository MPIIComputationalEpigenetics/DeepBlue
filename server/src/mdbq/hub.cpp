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
#include <mongo/client/gridfs.h>

#include "hub.hpp"

#include "../log.hpp"

#include "../dba/collections.hpp"
#include "../dba/config.hpp"
#include "../dba/helpers.hpp"
#include "../dba/users.hpp"

#include "../extras/date_time.hpp"
#include "../extras/utils.hpp"

#include "../connection/connection.hpp"


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

namespace epidb {
  namespace mdbq {
    struct HubImpl {
      epidb::Connection m_con;

      unsigned int m_interval;
      std::string  m_prefix;
      std::auto_ptr<boost::asio::deadline_timer> m_timer;

      void update_check(Hub *c, const boost::system::error_code &error)
      {
        // search for jobs which have failed and reschedule them
        mongo::BSONObj ret = BSON("_id" << 1 <<
                                  "owner" << 1 <<
                                  "nfailed" << 1);
        auto p =
          m_con->query( dba::helpers::collection_name(dba::Collections::JOBS()),
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

          m_con->update(dba::helpers::collection_name(dba::Collections::JOBS()),
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
      m_ptr->m_con->createCollection(dba::helpers::collection_name(dba::Collections::JOBS()));
      m_ptr->m_prefix = prefix;
    }

    bool Hub::exists_job(const mongo::BSONObj &job, std::string &id)
    {
      epidb::Connection c;
      mongo::BSONObj ret = c->findOne(dba::helpers::collection_name(dba::Collections::JOBS()), BSON("misc" << job));
      c.done();

      if (ret.isEmpty()) {
        return false;
      } else {
        id = ret["_id"].String();
        return true;
      }
    }

    bool Hub::insert_job(const mongo::BSONObj &job, unsigned int timeout, const int version_value, std::string &id, std::string &msg)
    {
      int r_id;
      if (!dba::helpers::get_increment_counter("request", r_id, msg))  {
        return false;
      }

      id = "r" + epidb::utils::integer_to_string(r_id);

      boost::posix_time::ptime ctime = epidb::extras::universal_date_time();
      m_ptr->m_con->insert(dba::helpers::collection_name(dba::Collections::JOBS()),
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
      return m_ptr->m_con->count(dba::helpers::collection_name(dba::Collections::JOBS()), BSON( "state" << TS_NEW));
    }

    size_t Hub::get_n_assigned()
    {
      return m_ptr->m_con->count(dba::helpers::collection_name(dba::Collections::JOBS()), BSON( "state" << TS_RUNNING));
    }

    size_t Hub::get_n_ok()
    {
      return m_ptr->m_con->count(dba::helpers::collection_name(dba::Collections::JOBS()), BSON( "state" << TS_DONE));
    }

    size_t Hub::get_n_failed()
    {
      return m_ptr->m_con->count(dba::helpers::collection_name(dba::Collections::JOBS()), BSON( "state" << TS_FAILED));
    }

    void Hub::clear_all()
    {
      m_ptr->m_con->dropCollection(dba::helpers::collection_name(dba::Collections::JOBS()));
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
      return m_ptr->m_con->findOne(dba::helpers::collection_name(dba::Collections::JOBS()), BSON("_id" << id));
    }

    std::list<mongo::BSONObj> Hub::get_jobs(const mdbq::TaskState& state, const std::string &user_id)
    {
      std::list<mongo::BSONObj> ret;

      if (state == mdbq::_TS_END) {
        auto cursor = m_ptr->m_con->query(dba::helpers::collection_name(dba::Collections::JOBS()),
                                          BSON("misc.user_id" << user_id));
        while (cursor->more()) {
          mongo::BSONObj o = cursor->next().getOwned();
          ret.push_back(o);
        }

      } else {
        auto cursor = m_ptr->m_con->query(dba::helpers::collection_name(dba::Collections::JOBS()),
                                          BSON("state" << state << "misc.user_id" << user_id));
        while (cursor->more()) {
          mongo::BSONObj o = cursor->next().getOwned();
          ret.push_back(o);
        }
      }

      return ret;
    }

    bool Hub::job_has_user_id(const std::string& request_id, const std::string& user_id)
    {
      return m_ptr->m_con->count(dba::helpers::collection_name(dba::Collections::JOBS()),
                                 BSON("_id" << request_id << "misc.user_id" << user_id)) > 0;
    }

    mongo::BSONObj Hub::get_newest_finished()
    {
      return m_ptr->m_con->findOne(dba::helpers::collection_name(dba::Collections::JOBS()),
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
      } else if (name == "canceled") {
        return TS_CANCELLED;
      } else if (name == "removed") {
        return TS_REMOVED;
      } else if (name == "renew") {
        return TS_RENEW;
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
      case TS_CANCELLED:
        return "canceled";
      case TS_REMOVED:
        return "removed";
      case TS_RENEW:
        return "renew";
      default :
        return "Invalid State: " + epidb::utils::integer_to_string(state);
      }
    }

    std::string Hub::state_message(const mongo::BSONObj &o)
    {
      int state = o["state"].Int();

      switch (state) {
      case TS_NEW:
      case TS_RUNNING:
      case TS_DONE:
      case TS_CANCELLED:
      case TS_REMOVED:
      case TS_RENEW:
        return "";
      case TS_FAILED:
        return o["error"].str();
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
      TaskState task_state = (TaskState) o["state"].Int();
      return task_state == TS_DONE;
    }

    bool Hub::is_failed(const mongo::BSONObj &o)
    {
      TaskState task_state = (TaskState) o["state"].Int();
      return task_state == TS_FAILED;
    }

    bool Hub::get_file_info(const std::string &filename, mongo::OID &oid, size_t &chunk_size, size_t &file_size, std::string &msg)
    {
      auto data_cursor = m_ptr->m_con->query(m_ptr->m_prefix + ".fs.files", mongo::Query(BSON("filename" << filename)));

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
        auto data_cursor = m_ptr->m_con->query(m_ptr->m_prefix + ".fs.chunks", q, 0, 0, &projection);
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

    bool Hub::remove_request_data(const std::string& request_id, TaskState state, std::string& msg)
    {
      // This function has 3 steps:
      // 1. Mark the request as removed.
      // 2. Mark the processing as removed, but keep it for future reference.
      // 3. Delete the data from gridfs

      // 1.
      epidb::Connection c;
      boost::posix_time::ptime now = epidb::extras::universal_date_time();
      mongo::BSONObj cmd = BSON(
                             "findAndModify" << dba::Collections::JOBS() <<
                             "query" << BSON("_id" << request_id) <<
                             "update" << BSON("$set" <<
                                              BSON("state" << state
                                                  << "refresh_time" << epidb::extras::to_mongo_date(now)
                                                  )
                                             )
                           );
      mongo::BSONObj res;
      c->runCommand(m_ptr->m_prefix, cmd, res);
      if (!res["value"].isABSONObj()) {
        msg =  "No request available, cmd:" + cmd.toString();
        return false;
      }


      // 2.
      mongo::BSONObj query = BSON("request_id" << request_id);
      mongo::BSONObj update = BSON("$set" << BSON("status" << TS_REMOVED));
      c->update(epidb::dba::helpers::collection_name(dba::Collections::PROCESSING()), query, update, false, true);
      if (!m_ptr->m_con->getLastError().empty()) {
        msg = c->getLastError();
        c.done();
        return false;
      }

      // 3.
      remove_result(request_id);

      return true;
    }

    bool Hub::cancel_request(const std::string& request_id, std::string& msg)
    {
      boost::posix_time::ptime now = epidb::extras::universal_date_time();

      mongo::BSONObjBuilder queryb;
      mongo::BSONObj res, cmd, query;
      queryb.append("_id", request_id);

      query = queryb.obj();

      cmd = BSON(
              "findAndModify" << dba::Collections::JOBS() <<
              "query" << query <<
              "update" << BSON("$set" <<
                               BSON("finish_time" << epidb::extras::to_mongo_date(now)
                                    << "state" << TS_CANCELLED
                                    << "refresh_time" << epidb::extras::to_mongo_date(now)
                                   )
                              )
            );

      epidb::Connection c;
      c->runCommand(m_ptr->m_prefix, cmd, res);


      if (!res["value"].isABSONObj()) {
        std::cout << "No task available, cmd:" << cmd << std::endl;
        return false;
      }

      mongo::BSONObj task_info = res["value"].Obj();
      TaskState task_state = static_cast<TaskState>(task_info["state"].Int());

      if (task_state == TS_DONE || task_state == TS_FAILED) {
        remove_request_data(request_id, TS_REMOVED, msg);
      }
      return true;
    }

    void Hub::remove_result(const std::string request_id)
    {
      Connection c;
      EPIDB_LOG_DBG("Removing data for the request: " + request_id);
      mongo::GridFS gridfs(c.conn(), dba::config::DATABASE_NAME(), "fs");
      gridfs.removeFile(request_id);
    }

  }
}
