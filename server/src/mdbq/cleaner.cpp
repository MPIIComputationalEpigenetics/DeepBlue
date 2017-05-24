//
//  cleaner.hpp
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

#include <mongo/client/dbclient.h>

#include "../connection/connection.hpp"

#include "../datatypes/user.hpp"

#include "../dba/collections.hpp"
#include "../dba/config.hpp"
#include "../dba/helpers.hpp"

#include "../extras/date_time.hpp"
#include "../extras/utils.hpp"

#include "common.hpp"
#include "cleaner.hpp"

#include "../errors.hpp"
#include "../log.hpp"

namespace epidb {
  namespace mdbq {
    bool remove_request_data(const std::string& request_id, TaskState state, std::string& msg)
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
      c->runCommand(dba::config::DATABASE_NAME(), cmd, res);
      if (!res["value"].isABSONObj()) {
        c.done();
        msg =  "No request available, cmd:" + cmd.toString();
        return false;
      }


      // 2.
      mongo::BSONObj query = BSON("request_id" << request_id);
      mongo::BSONObj update = BSON("$set" << BSON("status" << state));
      c->update(epidb::dba::helpers::collection_name(dba::Collections::PROCESSING()), query, update, false, true);
      if (!c->getLastError().empty()) {
        msg = c->getLastError();
        c.done();
        return false;
      }

      // 3.
      remove_result(request_id);

      c.done();
      return true;
    }

    bool cancel_request(const datatypes::User& user, const std::string& request_id, std::string& msg)
    {
      boost::posix_time::ptime now = epidb::extras::universal_date_time();

      mongo::BSONObjBuilder queryb;
      mongo::BSONObj res, cmd, query;
      queryb.append("_id", request_id);

      if (!user.is_admin()) {
        queryb.append("misc.user_id", user.get_id());
      }

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
      c->runCommand(dba::config::DATABASE_NAME(), cmd, res);

      if (!res["value"].isABSONObj()) {
        msg = Error::m(ERR_REQUEST_ID_INVALID, request_id);
        c.done();
        return false;
      }

      mongo::BSONObj task_info = res["value"].Obj();
      TaskState task_state = static_cast<TaskState>(task_info["state"].numberInt());

      if (task_state == TS_DONE || task_state == TS_FAILED) {
        remove_request_data(request_id, TS_REMOVED, msg);
      }
      return true;
    }

    void remove_result(const std::string request_id)
    {
      Connection c;
      EPIDB_LOG_DBG("Removing data for the request: " + request_id);
      mongo::GridFS gridfs(c.conn(), dba::config::DATABASE_NAME(), "fs");
      gridfs.removeFile(request_id);
      c.done();
    }

  }
}