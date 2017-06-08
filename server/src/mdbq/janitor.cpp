//
//  janitor.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 24.05.17.
//  Copyright (c) 2016 Max Planck Institute for Informatics. All rights reserved.

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

#include <boost/asio.hpp>

#include <mongo/bson/bson.h>
#include <mongo/client/dbclient.h>

#include "../config/config.hpp"

#include "../connection/connection.hpp"

#include "../dba/collections.hpp"
#include "../dba/helpers.hpp"

#include "../extras/date_time.hpp"

#include "janitor.hpp"
#include "common.hpp"
#include "cleaner.hpp"

#include "../log.hpp"

namespace epidb {
  namespace mdbq {

    void Janitor::run()
    {
      EPIDB_LOG_TRACE("Starting Janitor");
      reg(ios);
      ios.run();
    }

    void Janitor::reg(boost::asio::io_service &io_service)
    {
      unsigned long long interval = epidb::config::get_janitor_periodicity();

      m_timer.reset(new boost::asio::deadline_timer(io_service,
                    boost::posix_time::seconds(interval)));
      m_timer->async_wait(boost::bind(&Janitor::clean_oldest, this, boost::asio::placeholders::error));
    }

    void Janitor::update(config::ConfigSubject & subject)
    {
      reset();
    }

    void Janitor::reset()
    {
      m_timer->cancel();
      unsigned long long interval = epidb::config::get_janitor_periodicity();
      m_timer.reset(new boost::asio::deadline_timer(ios, boost::posix_time::seconds(interval)));
      m_timer->async_wait(boost::bind(&Janitor::clean_oldest, this, boost::asio::placeholders::error));
    }

    bool Janitor::clean_oldest(const boost::system::error_code &error)
    {
      if (error == boost::asio::error::operation_aborted) {
        EPIDB_LOG_DBG("clean_oldest canceled, probably it was reset");
        return false;
      }

      mongo::BSONObjBuilder bob;

      boost::posix_time::ptime prev = epidb::extras::time_ago(config::get_old_request_age_in_sec());

      bob.append("state", TaskState::TS_DONE);
      bob.append("$or", BSON_ARRAY(BSON("misc.command" << "score_matrix") << BSON("misc.command" << "get_regions")));
      bob.append("finish_time", BSON("$lte" << epidb::extras::to_mongo_date(prev)));

      mongo::Query query = mongo::Query(bob.obj()).sort(BSON("finish_time" << 1));

      Connection c;
      auto cursor = c->query(dba::helpers::collection_name(dba::Collections::JOBS()), query, 1);

      while (cursor->more()) {
        const mongo::BSONObj job = cursor->next();
        EPIDB_LOG_DBG("CLEARING JOB " << job.toString());
        const std::string& id = job["_id"].String();

        std::string msg;

        bool o = remove_request_data(id, TS_CLEARED, msg);
        if (o) {
          EPIDB_LOG_DBG("Job " << id << " cleared with success");
        } else {
          EPIDB_LOG_DBG("Problem clearing job: " << id << " - ");
        }
      }
      c.done();

      unsigned long long interval = epidb::config::get_janitor_periodicity();
      m_timer->expires_at(m_timer->expires_at() + boost::posix_time::seconds(interval));
      m_timer->async_wait(boost::bind(&Janitor::clean_oldest, this, boost::asio::placeholders::error));

      return true;
    }
  }
}