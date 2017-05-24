//
//  janitor.hpp
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

#include "../connection/connection.hpp"

#include "../dba/collections.hpp"
#include "../dba/helpers.hpp"

#include "janitor.hpp"

#include "common.hpp"

#include "../log.hpp"

namespace epidb {
  namespace mdbq {

    void Janitor::run() {
      EPIDB_LOG_TRACE("Starting Janitor");
      reg(ios, this->m_interval);
      ios.run();
    }

    void Janitor::reg(boost::asio::io_service &io_service, float interval)
    {
      m_interval = interval;
      m_timer.reset(new boost::asio::deadline_timer(io_service,
                           boost::posix_time::seconds(interval) +
                           boost::posix_time::millisec((int)(1000 * (interval - (int)interval)))));
      m_timer->async_wait(boost::bind(&Janitor::clean_oldest, this, boost::asio::placeholders::error));
    }

    bool Janitor::clean_oldest(const boost::system::error_code &error)
    {
      EPIDB_LOG_TRACE("CLEAN OLDEST");
      // {state: 2, "$or":[{"misc.command":"get_regions"}, {"misc.command":"score_matrix"}]}).sort({finish_time: 1}).limit(1)
      mongo::BSONObjBuilder bob;

      bob.append("state", TaskState::TS_DONE);
      bob.append("$or", BSON_ARRAY(BSON("misc.command" << "score_matrix") << BSON("misc.command" << "get_regions")));

      mongo::Query query = mongo::Query(bob.obj()).sort(BSON("finish_time" << 1));

      Connection c;
      auto cursor = c->query(dba::helpers::collection_name(dba::Collections::JOBS()), query, 1);

      EPIDB_LOG_TRACE(query.toString());

      while (cursor->more()) {
        mongo::BSONObj job = cursor->next();
        std::cerr << "CLEARING JOB " << job.toString() << std::endl;
      }
      c.done();

      m_timer->expires_at(m_timer->expires_at() + boost::posix_time::millisec(1000 * this->m_interval));
      m_timer->async_wait(boost::bind(&Janitor::clean_oldest, this, boost::asio::placeholders::error));

      return true;
    }
  }
}