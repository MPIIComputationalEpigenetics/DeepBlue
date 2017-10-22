//
//  processing.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 02.06.15.
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

#include <atomic>
#include <chrono>
#include <memory>
#include <string>

#include "../connection/connection.hpp"
#include "../dba/collections.hpp"
#include "../dba/helpers.hpp"
#include "../engine/engine.hpp"
#include "../extras/utils.hpp"
#include "../mdbq/common.hpp"

#include "processing.hpp"
#include "running_cache.hpp"

#include "../extras/date_time.hpp"

#include "../errors.hpp"

namespace epidb {

  namespace processing {

    const std::map<OP, std::string> create_OP_names_map()
    {
      std::map<OP, std::string> m;

      m[PROCESS_QUERY] = "Process query";
      m[GET_EXPERIMENT_BY_QUERY] =  "Get experiments by query";
      m[COUNT_REGIONS] = "Count regions";
      m[RETRIEVE_EXPERIMENT_SELECT_QUERY] = "Retrieve experiments data";
      m[RETRIEVE_ANNOTATION_SELECT_QUERY] = "Retrieve annotations data";
      m[RETRIEVE_INTERSECTION_QUERY] = "Intersections";
      m[RETRIEVE_OVERLAP_QUERY] = "Overlaps";
      m[RETRIEVE_MERGE_QUERY] = "Merge";
      m[RETRIEVE_FIND_MOTIF_QUERY] = "Find motif";
      m[RETRIEVE_FILTER_QUERY] = "Filtering";
      m[RETRIEVE_TILING_QUERY] = "Tiling regions";
      m[RETRIEVE_GENES_DATA] = "Getting genes data";
      m[PROCESS_AGGREGATE] = "Aggregate";
      m[PROCESS_DISTINCT] = "Process distinct";
      m[PROCESS_BINNING] = "Process binning";
      m[PROCESS_CALCULATE_ENRICHMENT] = "Process calculate enrichment";
      m[PROCESS_COUNT] = "Process count";
      m[PROCESS_COVERAGE] = "Process coverage";
      m[PROCESS_GET_EXPERIMENTS_BY_QUERY] = "Process get experiments by query";
      m[PROCESS_GET_REGIONS] = "Process get regions";
      m[PROCESS_SCORE_MATRIX] = "Process score matrix";
      m[PROCESS_LOLA] = "Process LOLA";
      m[RETRIEVE_QUERY_REGION_SET] = "Retrieve query regions set";
      m[RETRIEVE_FLANK_QUERY] = "Flanking";
      m[RETRIEVE_EXPRESSIONS_DATA] = "Getting expressions data";
      m[FORMAT_OUTPUT] = "Formating output file";

      return m;
    }

    std::string& op_name(const OP& op)
    {
      return OP_names[op];
    }

    RunningOp::RunningOp(const std::string& processing_id, const OP& op, const mongo::BSONObj& param):
      _id(mongo::OID::gen()),
      _processing_id(processing_id),
      _op(op),
      _start_time(extras::universal_date_time())
    {
      Connection c;
      mongo::BSONObj insert = BSON("_id" << _id <<
                                   "p_id" << _processing_id <<
                                   "op" << _op <<
                                   "op_name" << op_name(op) <<
                                   "params" << param <<
                                   "s" << extras::to_mongo_date(_start_time));

      c->insert(dba::helpers::collection_name(dba::Collections::PROCESSING_OPS()), insert);
      c.done();
    }

    RunningOp::~RunningOp()
    {
      Connection c;
      boost::posix_time::ptime now = extras::universal_date_time();
      boost::posix_time::time_duration  total = now - _start_time;
      mongo::BSONObj query = BSON("_id" << _id);
      mongo::BSONObj update_value = BSON("$set" << BSON("e" << extras::to_mongo_date(now) << "t" << (long long) total.total_milliseconds()));
      c->update(dba::helpers::collection_name(dba::Collections::PROCESSING_OPS()), query, update_value, false, false);
      c.done();
    }


    Status::Status(const std::string &request_id, const long long maximum_memory) :
      _request_id(request_id),
      _maximum_memory(maximum_memory),
      _canceled(false),
      _total_regions(0),
      _total_size(0),
      _total_stored_data(0),
      _total_stored_data_compressed(0),
      _last_update(std::chrono::duration_cast< std::chrono::seconds >( std::chrono::system_clock::now().time_since_epoch())),
      _update_time_out(1),
      _running_cache(std::unique_ptr<RunningCache>(new RunningCache()))
    {
      if (_request_id != DUMMY_REQUEST) {
        std::string msg;
        int id;
        dba::helpers::get_increment_counter("processing", id, msg);
        dba::helpers::notify_change_occurred("processing", msg);
        _processing_id = "pc" + utils::integer_to_string(id);

        Connection c;
        mongo::BSONObj b = BSON("_id" << _processing_id << "request_id" << _request_id);
        c->insert(dba::helpers::collection_name(dba::Collections::PROCESSING()), b);
        c.done();
      }
    }

    Status::~Status()
    {
      if (_request_id != DUMMY_REQUEST) {
        Connection c;
        mongo::BSONObj query = BSON("_id" << _processing_id);
        mongo::BSONObj update_value = BSON("$set" << BSON("total_regions" << (long long) _total_regions.load() << "total_size" << (long long) _total_size.load()));
        c->update(dba::helpers::collection_name(dba::Collections::PROCESSING()), query, update_value, false, false);
        c.done();
      }
    }

    RunningOp Status::start_operation(OP op, const mongo::BSONObj& params)
    {
      return RunningOp(_processing_id, op, params);
    }

    void Status::update_values_in_db()
    {
      auto current_second = std::chrono::duration_cast< std::chrono::seconds >(std::chrono::system_clock::now().time_since_epoch());
      if (current_second - _last_update > _update_time_out) {
        Connection c;
        mongo::BSONObj query = BSON("_id" << _processing_id);
        mongo::BSONObj update_value = BSON("$set" <<
                                           BSON("total_regions" << (long long) _total_regions.load() <<
                                                "total_size" << (long long) _total_size.load())
                                          );
        c->update(dba::helpers::collection_name(dba::Collections::PROCESSING()), query, update_value, false, false);
        c.done();
        _last_update = current_second;
      }

    }

    void Status::sum_regions(const long long qtd)
    {
      _total_regions += qtd;
      update_values_in_db();
    }

    void Status::subtract_regions(const long long qtd)
    {
      _total_regions -= qtd;
      update_values_in_db();
    }

    long long Status::sum_size(const long long size)
    {
      _total_size += size;
      update_values_in_db();

      return _total_size;
    }

    long long Status::subtract_size(const long long size)
    {
      _total_size -= size;
      update_values_in_db();

      return _total_size;
    }

    void Status::set_total_stored_data(const long long size)
    {
      _total_stored_data = size;

      Connection c;
      mongo::BSONObj query = BSON("_id" << _processing_id);
      mongo::BSONObj update_value = BSON("$set" << BSON("total_stored_data" << (long long) _total_stored_data.load()));
      c->update(dba::helpers::collection_name(dba::Collections::PROCESSING()), query, update_value, false, false);
      c.done();
    }

    void Status::set_total_stored_data_compressed(const long long size)
    {
      _total_stored_data_compressed = size;

      Connection c;
      mongo::BSONObj query = BSON("_id" << _processing_id);
      mongo::BSONObj update_value = BSON("$set" << BSON("total_stored_data_compressed" << (long long) _total_stored_data_compressed.load()));
      c->update(dba::helpers::collection_name(dba::Collections::PROCESSING()), query, update_value, false, false);
      c.done();
    }

    bool Status::is_allowed_size(size_t output_size)
    {
      if (_request_id == DUMMY_REQUEST) {
        return true;
      }
      return (output_size <= static_cast<size_t>(_maximum_memory));
    }

    long long Status::total_regions()
    {
      return _total_regions;
    }

    long long Status::total_size()
    {
      return _total_size;
    }

    long long Status::maximum_size()
    {
      return _maximum_memory;
    }

    boost::posix_time::ptime last_checked;
    bool Status::is_canceled(bool& ret, std::string& msg)
    {
      auto current_second = std::chrono::duration_cast< std::chrono::seconds >(std::chrono::system_clock::now().time_since_epoch());
      if (current_second - _last_update > _update_time_out) {
        mongo::BSONObj result;

        if (!dba::helpers::get_one(dba::Collections::JOBS(),
                                   mongo::Query(BSON("_id" << _request_id)), result)) {
          msg = Error::m(ERR_REQUEST_CANCELED);
          return false;
        }
        _canceled = (result["state"].Int() == mdbq::TS_CANCELLED);
      }
      ret = _canceled;

      return true;
    }

    std::unique_ptr<RunningCache>& Status::running_cache()
    {
      return _running_cache;
    }

    typedef std::shared_ptr<Status> StatusPtr;

    StatusPtr build_status(const std::string& _id, const long long maximum_memory)
    {
      return std::make_shared<Status>(_id, maximum_memory);
    }

    StatusPtr build_dummy_status()
    {
      return std::make_shared<Status>(DUMMY_REQUEST, 0);
    }

    std::string DUMMY_REQUEST = "dummy";
    std::map<OP, std::string> OP_names = create_OP_names_map();
  }
}
