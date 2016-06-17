//
//  engine.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 08.04.14.
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

#include <regex>
#include <string>
#include <sstream>
#include <vector>

#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/bzip2.hpp>
#include <boost/iostreams/stream.hpp>

#include "commands.hpp"
#include "engine.hpp"

#include "../dba/config.hpp"
#include "../dba/queries.hpp"
#include "../dba/users.hpp"

#include "../extras/stringbuilder.hpp"
#include "../extras/utils.hpp"

#include "../processing/processing.hpp"

#include "../mdbq/common.hpp"

#include "../log.hpp"
#include "../version.hpp"

namespace epidb {

  Engine::Engine()
    : _hub(dba::config::get_mongodb_server(), dba::config::DATABASE_NAME())
  {
    EPIDB_LOG("Creating Engine");
  }

  bool Engine::init()
  {
    _hub.clear_all();

    return true;
  }

  bool Engine::execute(const std::string &name, const std::string &ip, unsigned long long id,
                       serialize::Parameters &parameters, serialize::Parameters &result) const
  {
    const Command *command = Command::get_command(name);
    if (command == 0) {
      std::stringstream ss;
      ss << "Command " << name << " does not exists.";
      EPIDB_LOG("Request (" << id << ") from " << ip << ": " << ss.str());
      result.add_error(ss.str());
      return false;
    }

    std::string msg;
    if (!command->check_parameters(parameters, msg)) {
      EPIDB_LOG("Request (" << id << ") from " << ip << ": " << name << " with bad typed parameters (" <<
                parameters.string(true) << "). " << msg);
      result.add_error(msg);
      return false;
    }

    EPIDB_LOG("Request (" << id << ") from " << ip << ": " << name << " with (" <<  parameters.string(true) << ").");
    std::cerr << "VAI EXECUTAR" << std::endl;
    return command->run(ip, parameters, result);
  }

  bool Engine::queue(const mongo::BSONObj &job, unsigned int timeout, std::string &id, std::string &msg)
  {
    if (_hub.exists_job(job, id)) {
      return true;
    }

    return _hub.insert_job(job, timeout, Version::version_value(), id, msg);
  }

  bool Engine::queue_count_regions(const std::string &query_id, const std::string &user_key, std::string &id, std::string &msg)
  {
    utils::IdName user;
    if (!dba::users::get_user(user_key, user, msg)) {
      return false;
    }
    if (!queue(BSON("command" << "count_regions" << "query_id" << query_id << "user_id" << user.id), 60 * 60, id, msg)) {
      return false;
    }
    return true;
  }

  bool Engine::queue_coverage(const std::string &query_id, const std::string &genome, const std::string &user_key, std::string &id, std::string &msg)
  {
    utils::IdName user;
    if (!dba::users::get_user(user_key, user, msg)) {
      return false;
    }
    if (!queue(BSON("command" << "coverage" << "query_id" << query_id << "genome" << genome << "user_id" << user.id), 60 * 60, id, msg)) {
      return false;
    }
    return true;
  }

  bool Engine::queue_get_regions(const std::string &query_id, const std::string &output_format, const std::string &user_key, std::string &id, std::string &msg)
  {
    utils::IdName user;
    if (!dba::users::get_user(user_key, user, msg)) {
      return false;
    }
    if (!queue(BSON("command" << "get_regions" << "query_id" << query_id << "format" << output_format << "user_id" << user.id), 60 * 60, id, msg)) {
      return false;
    }
    return true;
  }

  bool Engine::queue_score_matrix(const std::vector<std::pair<std::string, std::string>> &experiments_formats, const std::string &aggregation_function, const std::string &regions_query_id, const std::string &user_key, std::string &id, std::string &msg)
  {
    mongo::BSONObjBuilder bob_formats;

    for (auto &exp_format : experiments_formats) {
      bob_formats.appendElements(BSON(exp_format.first << exp_format.second));
    }

    utils::IdName user;
    if (!dba::users::get_user(user_key, user, msg)) {
      return false;
    }

    if (!queue(BSON("command" << "score_matrix" << "experiments_formats" << bob_formats.obj() << "aggregation_function" << aggregation_function << "query_id" << regions_query_id << "user_id" << user.id), 60 * 60, id, msg)) {
      return false;
    }

    return true;
  }


  bool Engine::queue_get_experiments_by_query(const std::string &query_id, const std::string &user_key, std::string &request_id, std::string &msg)
  {
    utils::IdName user;
    if (!dba::users::get_user(user_key, user, msg)) {
      return false;
    }
    if (!queue(BSON("command" << "get_experiments_by_query" << "query_id" << query_id << "user_id" << user.id), 60 * 60, request_id, msg)) {
      return false;
    }
    return true;
  }


  bool Engine::request_status(const std::string &request_id, const std::string &user_key, request::Status &request_status, std::string &msg)
  {
    utils::IdName user;
    if (!dba::users::get_user(user_key, user, msg)) {
      return false;
    }
    mongo::BSONObj o = _hub.get_job(request_id); // TODO still check
    if (o.isEmpty()) {
      msg = "Request ID " + request_id + " not found.";
      return false;
    }

    request_status.state = mdbq::Hub::state_name(o);
    request_status.message = mdbq::Hub::state_message(o);
    return true;
  }

  request::Job Engine::get_job_info(mongo::BSONObj o)
  {
    request::Job job;
    request::Status status;

    status.state = mdbq::Hub::state_name(o);
    status.message = mdbq::Hub::state_message(o);
    job.status = status;

    job.create_time = mdbq::Hub::get_create_time(o);
    if (status.state == mdbq::Hub::state_name(mdbq::TS_DONE)) {
      job.finish_time = mdbq::Hub::get_finish_time(o);
    }

    const mongo::BSONObj &misc = mdbq::Hub::get_misc(o);
    job.command = misc["command"].String();
    job.user_id = misc["user_id"].String();
    job.query_id = misc["query_id"].String();

    if (misc.hasElement("format")) {
      job.misc["format"] = misc["format"].String();
    }

    if (misc.hasElement("aggregation_function")) {
      job.misc["aggregation_function"] = misc["aggregation_function"].String();
    }

    if (misc.hasElement("experiments_formats")) {
      const mongo::BSONObj& experiments_formats = misc["experiments_formats"].Obj();
      for ( auto i = experiments_formats.begin(); i.more(); ) {
        auto e = i.next();
        job.misc[std::string("experiment:") + e.fieldName()] = e.String();
      }
    }

    job._id = mdbq::Hub::get_id(o);

    return job;
  }

  bool Engine::request_job(const std::string & request_id, request::Job & job, std::string & msg)
  {
    mongo::BSONObj o = _hub.get_job(request_id);
    if (o.isEmpty()) {
      msg = "Request ID " + request_id + " not found.";
      return false;
    }
    job = get_job_info(o);

    return true;
  }

  bool Engine::request_jobs(const std::string & status_find, const std::string & user_key, std::vector<request::Job>& ret, std::string & msg)
  {
    utils::IdName user;
    if (!dba::users::get_user(user_key, user, msg)) {
      return false;
    }

    mdbq::TaskState task_state = mdbq::Hub::state_number(status_find);
    std::list<mongo::BSONObj> jobs_bson = _hub.get_jobs(task_state, user.id);
    for (auto &job_bson : jobs_bson) {
      ret.push_back(get_job_info(job_bson));
    }
    return true;
  }

  bool Engine::check_request(const std::string & request_id, const std::string & user_key, std::string& msg)
  {
    utils::IdName user;
    if (!dba::users::get_user(user_key, user, msg)) {
      return false;
    }

    mongo::BSONObj o = _hub.get_job(request_id); //TODO still check
    if (o.isEmpty()) {
      msg = "Request ID " + request_id + " not found.";
      return false;
    }

    if (mdbq::Hub::is_failed(o)) {
      msg = o["error"].str();
      return false;
    }

    if (!mdbq::Hub::is_done(o)) {
      msg = "Request ID " + request_id + " was not finished. Please, check its status.";
      return false;
    }

    return true;
  }


  bool Engine::request_download_data(const std::string & request_id, const std::string & user_key, std::string &request_data, std::string& msg)
  {
    if (!check_request(request_id, user_key, msg)) {
      return false;
    }

    mongo::BSONObj o = _hub.get_job(request_id); //TODO still check
    mongo::BSONObj result = o["result"].Obj();

    if (result.hasField("__file__")) {
      std::string file_name = result["__file__"].str();
      std::string file_content;
      // Get compressed file from mongo filesystem
      return _hub.get_result(file_name, request_data, msg);
    }

    msg = "Request ID " + request_id + " does not contain a file has result.";
    return false;
  }


  bool Engine::request_data(const std::string & request_id, const std::string & user_key, serialize::Parameters &request_data)
  {
    std::string msg;
    if (!check_request(request_id, user_key, msg)) {

      std::cerr << "msg" << std::endl;
      std::cerr << msg << std::endl;
      request_data.add_error(msg);
      return false;
    }

    mongo::BSONObj o = _hub.get_job(request_id);
    mongo::BSONObj result = o["result"].Obj();

    if (result.hasField("__id_names__")) {
      mongo::BSONObj id_names = result["__id_names__"].Obj();
      request_data.add_param(bson_to_parameters(id_names));
      return true;
    }

    if (result.hasField("__file__")) {
      std::string file_name = result["__file__"].str();
      std::string file_content;
      // Get compressed file from mongo filesystem
      if (!_hub.get_result(file_name, file_content, msg)) {
        request_data.add_error(msg);
        return false;
      }

      // uncompress
      std::istringstream inStream(file_content, std::ios::binary);
      std::stringstream outStream;
      boost::iostreams::filtering_streambuf< boost::iostreams::input> in;
      in.push( boost::iostreams::bzip2_decompressor());
      in.push( inStream );
      // copy output
      boost::iostreams::copy(in, outStream);

      request_data.add_string_content(outStream.str());
      return true;
    }

    request_data.add_param(bson_to_parameters(result));
    return true;
  }

  serialize::ParameterPtr element_to_parameter(const mongo::BSONElement& e)
  {

    switch ( e.type() ) {
    case mongo::NumberDouble: {
      return std::make_shared<serialize::SimpleParameter>(e.Double());
    }
    case mongo::NumberInt:
    case mongo::NumberLong: {
      return std::make_shared<serialize::SimpleParameter>((long long) e.Long());
    }
    case mongo::Object: {
      return bson_to_parameters(e.Obj());
    }
    case mongo::Array: {
      serialize::ParameterPtr p = std::make_shared<serialize::ListParameter>();
      std::vector<mongo::BSONElement> ees = e.Array();
      for (auto const& ee : ees) {
        p->add_child(element_to_parameter(ee));
      }
      return p;
    }
    default: {
      return std::make_shared<serialize::SimpleParameter>(e.String());
    }
    }
  }

  serialize::ParameterPtr bson_to_parameters(const mongo::BSONObj & o)
  {
    serialize::ParameterPtr parameter(new serialize::MapParameter());

    for (mongo::BSONObj::iterator it = o.begin(); it.more(); ) {
      mongo::BSONElement e = it.next();

      std::string fieldname = e.fieldName();

      if (!fieldname.compare(0, 2, "__")) {
        continue;
      }

      parameter->add_child(fieldname, element_to_parameter(e));
    }

    return parameter;
  }

  bool Engine::user_owns_request(const std::string & request_id, const std::string & user_id)
  {
    if (_hub.job_has_user_id(request_id, user_id)) {
      return true;
    }
    return false;
  }

  bool Engine::cancel_request(const std::string & request_id, std::string & msg)
  {
    return _hub.cancel_request(request_id, msg);
  }

  bool Engine::remove_request_data(const std::string & request_id, mdbq::TaskState state, std::string & msg)
  {
    return _hub.remove_request_data(request_id, state, msg);
  }
}