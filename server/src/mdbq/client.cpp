//
//  client.cpp
//  epidb
//
// Code from https://github.com/temporaer/MDBQ/
// adapted to DeepBlue by Felipe Albrecht on 22.01.2015

#include <chrono>
#include <random>
#include <vector>

#include <boost/format.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include <mongo/client/dbclient.h>
#include <mongo/client/gridfs.h>

#include "../engine/engine.hpp"
#include "../extras/date_time.hpp"

#include "client.hpp"
#include "common.hpp"

#include "../connection/connection.hpp"

#define CHECK_DB_ERR(CON)\
  {\
    std::string e = (CON)->getLastError();\
    if(!e.empty()){\
      throw std::runtime_error("MDBQC: error_code!=0, failing: " + e + "\n" + (CON)->getLastErrorDetailed().toString() );\
    }\
  }

namespace mdbq {

  struct ClientImpl {
    epidb::Connection    m_con;
    mongo::BSONObj            m_current_task;
    mongo::BSONObj            m_task_selector;
    boost::posix_time::ptime  m_current_task_timeout_time;
    long long int             m_running_nr;
    //std::auto_ptr<mongo::BSONArrayBuilder>   m_log;
    std::vector<mongo::BSONObj> m_log;
    float              m_interval;
    std::auto_ptr<boost::asio::deadline_timer> m_timer;
    void update_check(Client *c, const boost::system::error_code &error)
    {
      std::string _id;
      mongo::BSONObj task;
      if (c->get_next_task(_id, task))
        c->handle_task(_id, task);
      if (!error) {
        unsigned int ms;
        if (m_interval <= 1.f)
          ms = 1000 * (m_interval / 2 + drand48() * (m_interval / 2));
        else
          ms = 1000 * (1 + drand48() * (m_interval - 1));
        m_timer->expires_at(m_timer->expires_at() + boost::posix_time::millisec(ms));
        m_timer->async_wait(boost::bind(&ClientImpl::update_check, this, c, boost::asio::placeholders::error));
      }
    }
  };

  Client::Client(const std::string &url, const std::string &prefix)
    : m_jobcol(prefix + ".jobs")
    , m_logcol(prefix + ".log")
    , m_fscol(prefix + ".fs")
    , m_verbose(false)
  {
    m_ptr.reset(new ClientImpl());
    CHECK_DB_ERR(m_ptr->m_con);

    m_db = prefix;
  }

  Client::Client(const std::string &url, const std::string &prefix, const mongo::BSONObj &query)
    : m_jobcol(prefix + ".jobs")
    , m_logcol(prefix + ".log")
    , m_fscol(prefix + ".fs")
    , m_verbose(false)
  {
    m_ptr.reset(new ClientImpl());
    m_ptr->m_task_selector = query;
    CHECK_DB_ERR(m_ptr->m_con);

    m_db = prefix;
  }

  bool Client::get_next_task(std::string& _id, mongo::BSONObj &o)
  {
    if (!m_ptr->m_current_task.isEmpty()) {
      throw std::runtime_error("MDBQC: do tasks one by one, please!");
    }
    boost::posix_time::ptime now = epidb::extras::universal_date_time();

    std::string hostname(256, '\0');
    gethostname(&hostname[0], 256);
    std::string hostname_pid = (boost::format("%s:%d") % &hostname[0] % getpid()).str();

    mongo::BSONObjBuilder queryb;
    mongo::BSONObj res, cmd, query;
    queryb.append("state", TS_NEW);
    if (! m_ptr->m_task_selector.isEmpty())
      queryb.appendElements(m_ptr->m_task_selector);
    query = queryb.obj();
    cmd = BSON(
            "findAndModify" << "jobs" <<
            "query" << query <<
            "update" << BSON("$set" <<
                             BSON("book_time" << epidb::extras::to_mongo_date(now)
                                  << "state" << TS_RUNNING
                                  << "refresh_time" << epidb::extras::to_mongo_date(now)
                                  << "owner" << hostname_pid)));

    m_ptr->m_con->runCommand(m_db, cmd, res);
    CHECK_DB_ERR(m_ptr->m_con);

    if (!res["value"].isABSONObj()) {
      if (m_verbose)
        std::cout << "No task available, cmd:" << cmd << std::endl;
      return false;
    }

    m_ptr->m_current_task = res["value"].Obj().copy();

    int timeout_s = INT_MAX;
    if (m_ptr->m_current_task.hasField("timeout"))
      timeout_s = m_ptr->m_current_task["timeout"].Int();

    m_ptr->m_current_task_timeout_time = now + boost::posix_time::seconds(timeout_s);
    m_ptr->m_running_nr = 0;

    _id  = m_ptr->m_current_task["_id"].str();
    o = m_ptr->m_current_task["misc"].Obj().getOwned();

    // start logging
    m_ptr->m_log.clear();
    return true;
  }

  void Client::finish(const mongo::BSONObj &result, bool ok)
  {
    const mongo::BSONObj &ct = m_ptr->m_current_task;
    if (ct.isEmpty()) {
      throw std::runtime_error("MDBQC: get a task first before you finish!");
    }
    boost::posix_time::ptime finish_time = epidb::extras::universal_date_time();
    int version = ct["version"].Int();
    if (ok) {
      m_ptr->m_con->update(m_jobcol,
                           BSON("_id" << ct["_id"] <<
                                "version" << version),
                           BSON("$set" << BSON(
                                  "state" << TS_DONE <<
                                  "version" << version + 1 <<
                                  "finish_time" << epidb::extras::to_mongo_date(finish_time) <<
                                  "result" << result)));
    } else {
      std::string tmp;
      mongo::BSONObj ret;
      ret = m_ptr->m_con->findOne(m_jobcol, BSON("_id" << ct["_id"]));
      if (ret.hasField("state") &&
          (ret["state"].Int() == mdbq::TS_CANCELLED ||
           ret["state"].Int() == mdbq::TS_REMOVED)) {
        std::cout << "removed" << std::endl;
        epidb::Engine::instance().remove_request_data(ct["_id"], tmp);
      } else {
        m_ptr->m_con->update(m_jobcol,
                             BSON("_id" << ct["_id"] <<
                                  "version" << version),
                             BSON("$set" << BSON(
                                    "state" << TS_FAILED <<
                                    "version" << version + 1 <<
                                    "failure_time" << epidb::extras::to_mongo_date(finish_time) <<
                                    "error" << result["__error__"].str())));
      }
    }
    CHECK_DB_ERR(m_ptr->m_con);
    m_ptr->m_current_task = mongo::BSONObj(); // empty, call get_next_task.
  }

  void Client::reg(boost::asio::io_service &io_service, float interval)
  {
    m_ptr->m_interval = interval;
    m_ptr->m_timer.reset(new boost::asio::deadline_timer(io_service,
                         boost::posix_time::seconds(interval) +
                         boost::posix_time::millisec((int)(1000 * (interval - (int)interval)))));
    m_ptr->m_timer->async_wait(boost::bind(&ClientImpl::update_check, m_ptr.get(), this, boost::asio::placeholders::error));
  }

  void Client::handle_task(const std::string& _id, const mongo::BSONObj &o)
  {
    std::cerr << "MDBQC: WARNING: got a task, but no handler defined!" << std::endl;
    finish(BSON("error" << true));
  }

  Client::~Client() { }

  std::string Client::store_result(const char *ptr, size_t len)
  {
    mongo::BSONObj &ct = m_ptr->m_current_task;
    if (ct.isEmpty()) {
      throw std::runtime_error("MDBQC: get a task first before you store something about it!");
    }

    std::string filename = ct["_id"].str();

    mongo::GridFS gridfs((*m_ptr).m_con.conn(), m_db, "fs");
    gridfs.setChunkSize(2 << 22); // 8MB
    mongo::BSONObj ret = gridfs.storeFile(ptr, len, filename);

    mongo::BSONObjBuilder bob;
    bob.appendElements(ret);
    m_ptr->m_con->update(m_fscol + ".files",
                         BSON("filename" << ret.getField("filename")),
                         bob.obj(), false, false);

    CHECK_DB_ERR(m_ptr->m_con);

    return filename;
  }

  std::vector<mongo::BSONObj> Client::get_log(const mongo::BSONObj &task)
  {
    std::auto_ptr<mongo::DBClientCursor> p =
      m_ptr->m_con->query( m_logcol,
                           mongo::Query(BSON("taskid" << task["_id"])).sort("nr"));
    CHECK_DB_ERR(m_ptr->m_con);
    std::vector<mongo::BSONObj> log;
    while (p->more()) {
      mongo::BSONObj f = p->next();
      log.push_back(f);
    }
    return log;
  }

}
