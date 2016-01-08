//
//  engine.hpp
//  epidb
//
//  Created by Felipe Albrecht on 29.05.13.
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
//

#ifndef EPIDB_ENGINE_ENGINE_HPP
#define EPIDB_ENGINE_ENGINE_HPP

#include <boost/noncopyable.hpp>

#include "commands.hpp"
#include "request.hpp"

#include "../mdbq/hub.hpp"

#include "../log.hpp"

namespace epidb {
  class Engine : private boost::noncopyable {
  private:
    mdbq::Hub _hub;

    Engine();
    Engine(Engine const &);
    void operator=(Engine const &);

    bool queue(const mongo::BSONObj &job, unsigned int timeout, std::string &request_id, std::string &msg);

    request::Job get_job_info(mongo::BSONObj o);

  public:
    static Engine &instance()
    {
      static Engine instance;
      return instance;
    }

    bool init();

    bool execute(const std::string &name, const std::string &ip, unsigned long long id,
                 serialize::Parameters &parameters, serialize::Parameters &result) const;

    bool request_status(const std::string &request_id, const std::string &user_key, request::Status &status, std::string &msg);

    /*
    * \brief Get job with given id
    * \param    job     Return: Requested job
    */
    bool request_job(const std::string& request_id, request::Job& job, std::string &msg);

    /*
    * \brief Get all jobs having given status owned by given user. If no status is given, it returns all jobs by given user.
    * \param    status     Status to search for
    *           user_key   Key of the owning user
    *           ret        Return: All requested jobs
    */
    bool request_jobs(const std::string& status, const std::string& user_key, std::vector<request::Job>& ret, std::string &msg);

    bool request_data(const std::string &request_id, const std::string &user_key, request::Data &data, std::string& content, request::DataType& type, std::string &msg);

    bool queue_count_regions(const std::string &query_id, const std::string &user_key, std::string &request_id, std::string &msg);

    bool queue_get_regions(const std::string &query_id, const std::string &output_format, const std::string &user_key, std::string &id, std::string &msg);

    bool queue_score_matrix(const std::vector<std::pair<std::string, std::string>> &experiments_formats, const std::string &aggregation_function, const std::string &regions_query_id, const std::string &user_key, std::string &request_id, std::string &msg);

    bool queue_get_experiments_by_query(const std::string &query_id, const std::string &user_key, std::string &request_id, std::string &msg);

    /*
    * \brief Returns whether given user owns given request
    * \return False also if request_id or user_id do not exist
    */
    bool user_owns_request(const std::string& request_id, const std::string& user_id);

    bool cancel_request(const std::string& request_id, std::string& msg);

    bool remove_request_data(const std::string& request_id, mdbq::TaskState state, std::string& msg);
  };
}

#endif
