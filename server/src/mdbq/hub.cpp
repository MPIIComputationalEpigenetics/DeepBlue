//
//  hub.cpp
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

#include <iomanip>

#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include <mongo/client/dbclient.h>

#include "hub.hpp"

#include "../config/config.hpp"

#include "../connection/connection.hpp"

#include "../dba/collections.hpp"

#include "../dba/helpers.hpp"
#include "../dba/users.hpp"

#include "../extras/date_time.hpp"
#include "../extras/utils.hpp"

#include "../errors.hpp"
#include "../log.hpp"

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
      unsigned int m_interval;
      std::string  m_prefix;
      std::auto_ptr<boost::asio::deadline_timer> m_timer;

      void update_check(Hub *c, const boost::system::error_code &error)
      {
        // search for jobs which have failed and reschedule them
        mongo::BSONObj ret = BSON("_id" << 1 <<
                                  "owner" << 1 <<
                                  "nfailed" << 1);
        Connection conn;
        auto p = conn->query( dba::helpers::collection_name(dba::Collections::JOBS()),
                              BSON(
                                "state" << TS_FAILED <<
                                "nfailed" << mongo::LT << 1), /* first time failure only */
                              0, 0, &ret);
        CHECK_DB_ERR_LOG(conn);
        while (p->more()) {
          mongo::BSONObj f = p->next();
          if (!f.hasField("nfailed"))
            continue;

          std::cerr << "HUB: warning: task `"
                    << f["_id"] << "' on `"
                    << f["owner"].String() << "' failed, rescheduling" << std::endl;

          conn->update(dba::helpers::collection_name(dba::Collections::JOBS()),
                       BSON("_id" << f["_id"]),
                       BSON(
                         "$inc" << BSON("nfailed" << 1) <<
                         "$set" << BSON(
                           "state"         << TS_NEW
                           << "book_time"   << mongo::Undefined
                           << "refresh_time" << mongo::Undefined)));
          CHECK_DB_ERR_LOG(conn);
        }

        conn.done();
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
      m_ptr->m_prefix = prefix;

      // Move out from the ctor
      Connection c;
      c->createCollection(dba::helpers::collection_name(dba::Collections::JOBS()));
      c.done();
    }

    bool Hub::exists_job(const mongo::BSONObj &job, std::string &id, bool update)
    {
      mongo::BSONObj ret;      
      epidb::Connection c;

      std::cerr << job.toString() << std::endl;
      
      ret = c->findOne(dba::helpers::collection_name(dba::Collections::JOBS()), job);
      c.done();

      if (ret.isEmpty()) {
        std::cerr << "not found" << std::endl;
        return false;
      } else {
        if (update && (is_cleared(ret) || is_canceled(ret))) {
          reprocess_job(ret);
        }
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
      Connection c;
      c->insert(dba::helpers::collection_name(dba::Collections::JOBS()),
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
      CHECK_DB_ERR_RETURN(c, msg);
      c.done();
      return true;
    }

    bool Hub::reprocess_job(const mongo::BSONObj &job)
    {
      const std::string& _id = job["_id"].str();

      boost::posix_time::ptime now = epidb::extras::universal_date_time();

      mongo::BSONObjBuilder queryb;
      mongo::BSONObj res, cmd, query;
      queryb.append("_id", _id);

      query = queryb.obj();
      cmd = BSON(
              "findAndModify" << dba::Collections::JOBS() <<
              "query" << query <<
              "update" << BSON("$set" <<
                               BSON("state" << TS_REPROCESS
                                    << "refresh_time" << epidb::extras::to_mongo_date(now)
                                   )
                              )
            );
      Connection c;
      c->runCommand(m_ptr->m_prefix, cmd, res);
      if (!res["value"].isABSONObj()) {
        c.done();
        std::cerr << "No request '" + _id + "' available, cmd:" + cmd.toString() << std::endl;
        return false;
      }

      c.done();
      return true;
    }


    size_t Hub::get_n_open()
    {
      Connection c;
      size_t count = c->count(dba::helpers::collection_name(dba::Collections::JOBS()), BSON( "state" << BSON("$in" << BSON_ARRAY(TS_NEW << TS_REPROCESS))));
      c.done();
      return count;
    }

    size_t Hub::get_n_assigned()
    {
      Connection c;
      size_t count = c->count(dba::helpers::collection_name(dba::Collections::JOBS()), BSON( "state" << TS_RUNNING));
      c.done();
      return count;
    }

    size_t Hub::get_n_ok()
    {
      Connection c;
      size_t count = c->count(dba::helpers::collection_name(dba::Collections::JOBS()), BSON( "state" << TS_DONE));
      c.done();
      return count;
    }

    size_t Hub::get_n_failed()
    {
      Connection c;
      size_t count = c->count(dba::helpers::collection_name(dba::Collections::JOBS()), BSON( "state" << TS_FAILED));
      c.done();
      return count;
    }

    void Hub::clear_all()
    {
      Connection c;
      c->dropCollection(dba::helpers::collection_name(dba::Collections::JOBS()));
      c->dropCollection(m_prefix + ".log");
      c->dropCollection(m_prefix + ".fs.chunks");
      c->dropCollection(m_prefix + ".fs.files");

      // this is from https://jira.mongodb.org/browse/SERVER-5323
      // c->createIndex(m_prefix + ".fs.chunks", BSON("files_id" << 1 << "n" << 1));

      c.done();
    }

    void Hub::reg(boost::asio::io_service &io_service, unsigned int interval)
    {
      m_ptr->m_interval = interval;
      m_ptr->m_timer.reset(new boost::asio::deadline_timer(io_service, boost::posix_time::seconds(interval)));
      m_ptr->m_timer->async_wait(boost::bind(&HubImpl::update_check, m_ptr.get(), this, boost::asio::placeholders::error));
    }

    mongo::BSONObj Hub::get_job(const std::string &id, bool update_access)
    {
      Connection c;
      mongo::BSONObj o = c->findOne(dba::helpers::collection_name(dba::Collections::JOBS()), BSON("_id" << id));

      if (update_access) {
        boost::posix_time::ptime now = epidb::extras::universal_date_time();

        mongo::BSONObj cmd = BSON(
                               "findAndModify" << dba::Collections::JOBS() <<
                               "query" << BSON("_id" << id) <<
                               "update" << BSON(
                                 "$set" << BSON( "last_access" << epidb::extras::to_mongo_date(now)) <<
                                 "$inc" << BSON("times_accessed" << 1)
                               )
                             );

        mongo::BSONObj res;
        c->runCommand(m_ptr->m_prefix, cmd, res);
        if (!res["value"].isABSONObj()) {
          c.done();
          std::cerr << "No request available, cmd:" + cmd.toString() << std::endl;
          return res;
        }
      }

      c.done();
      return o;
    }

    std::list<mongo::BSONObj> Hub::get_jobs(const mdbq::TaskState& state, const std::string &user_id)
    {
      std::list<mongo::BSONObj> ret;

      Connection c;
      if (state == mdbq::_TS_END) {
        auto cursor = c->query(dba::helpers::collection_name(dba::Collections::JOBS()),
                               BSON("misc.user_id" << user_id));
        while (cursor->more()) {
          mongo::BSONObj o = cursor->next().getOwned();
          ret.push_back(o);
        }

      } else {
        auto cursor = c->query(dba::helpers::collection_name(dba::Collections::JOBS()),
                               BSON("state" << state << "misc.user_id" << user_id));
        while (cursor->more()) {
          mongo::BSONObj o = cursor->next().getOwned();
          ret.push_back(o);
        }
      }

      c.done();
      return ret;
    }

    bool Hub::job_has_user_id(const std::string& request_id, const std::string& user_id)
    {

      Connection c;
      bool b = c->count(dba::helpers::collection_name(dba::Collections::JOBS()),
                        BSON("_id" << request_id << "misc.user_id" << user_id)) > 0;
      c.done();
      return b;
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
      } else if (name == "cleared") {
        return TS_CLEARED;
      } else if (name == "reprocess") {
        return TS_REPROCESS;
      } else {
        return _TS_END;
      }
    }

    std::string Hub::state_name(const mongo::BSONObj &o)
    {
      return state_name(o["state"].numberInt());
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
      case TS_CLEARED:
        return "cleared";
      case TS_REPROCESS:
        return "reprocess";
      default :
        return "Invalid State: " + epidb::utils::integer_to_string(state);
      }
    }

    std::string Hub::state_message(const mongo::BSONObj &o)
    {
      int state = o["state"].numberInt();

      switch (state) {
      case TS_NEW:
      case TS_RUNNING:
      case TS_DONE:
      case TS_CANCELLED:
      case TS_REMOVED:
      case TS_RENEW:
      case TS_CLEARED:
      case TS_REPROCESS:
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

    bool Hub::is(const mongo::BSONObj &o, const mdbq::TaskState state)
    {
      TaskState task_state = (TaskState) o["state"].numberInt();
      return task_state == state;
    }

    bool Hub::is_done(const mongo::BSONObj &o)
    {
      return Hub::is(o, TS_DONE);
    }

    bool Hub::is_failed(const mongo::BSONObj &o)
    {
      return Hub::is(o, TS_FAILED);
    }

    bool Hub::is_cleared(const mongo::BSONObj &o)
    {
      return Hub::is(o, TS_CLEARED);
    }

    bool Hub::is_canceled(const mongo::BSONObj &o)
    {
      return Hub::is(o, TS_CANCELLED);
    }
  }
}
