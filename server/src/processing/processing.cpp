//
//  processing.cpp
//  epidb
//
//  Created by Felipe Albrecht on 02.06.15.
//  Copyright (c) 2015 Max Planck Institute for Computer Science. All rights reserved.
//

#include <atomic>
#include <memory>
#include <string>

#include "../connection/connection.hpp"
#include "../dba/collections.hpp"
#include "../dba/helpers.hpp"
#include "../extras/utils.hpp"

#include "processing.hpp"

#include "../extras/date_time.hpp"

namespace epidb {

  namespace processing {

    const std::map<OP, std::string> create_OP_names_map()
    {
      std::map<OP, std::string> m;

      m[GET_EXPERIMENT_BY_QUERY] =  "Get experiments by query";
      m[COUNT_REGIONS] = "Count regions";
      m[RETRIEVE_EXPERIMENT_SELECT_QUERY] = "Retrieve experiments data";
      m[RETRIEVE_ANNOTATION_SELECT_QUERY] = "Retrieve annotations data";
      m[RETRIEVE_INTERSECTION_QUERY] = "Intersections";
      m[RETRIEVE_MERGE_QUERY] = "Merge";
      m[RETRIEVE_FILTER_QUERY] = "Filtering";
      m[RETRIEVE_TILING_QUERY] = "Tiling regions";
      m[PROCESS_AGGREGATE] = "Aggregate";

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
      mongo::BSONObj update_value = BSON("$set" << BSON("e" << extras::to_mongo_date(now) << "t" << total. total_milliseconds()));
      c->update(dba::helpers::collection_name(dba::Collections::PROCESSING_OPS()), query, update_value, false, false);
      c.done();
    }


    Status::Status(const std::string &request_id) :
      _request_id(request_id),
      _total_regions(0),
      _total_size(0),
      _total_stored_data(0),
      _total_stored_data_compressed(0)
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

    void Status::sum_regions(size_t qtd)
    {
      _total_regions += qtd;

      if (qtd > _total_regions / 5 ) {
        Connection c;
        mongo::BSONObj query = BSON("_id" << _processing_id);
        mongo::BSONObj update_value = BSON("$set" << BSON("total_regions" << (long long) _total_regions.load()));
        c->update(dba::helpers::collection_name(dba::Collections::PROCESSING()), query, update_value, false, false);
        c.done();
      }
    }

    void Status::sum_size(size_t size)
    {
      _total_size += size;

      if (size > _total_size / 5) {
        Connection c;
        mongo::BSONObj query = BSON("_id" << _processing_id);
        mongo::BSONObj update_value = BSON("$set" << BSON("total_size" << (long long) _total_size.load()));
        c->update(dba::helpers::collection_name(dba::Collections::PROCESSING()), query, update_value, false, false);
        c.done();
      }
    }

    void Status::set_total_stored_data(size_t size)
    {
      _total_stored_data = size;

      Connection c;
      mongo::BSONObj query = BSON("_id" << _processing_id);
      mongo::BSONObj update_value = BSON("$set" << BSON("total_stored_data" << (long long) _total_stored_data.load()));
      c->update(dba::helpers::collection_name(dba::Collections::PROCESSING()), query, update_value, false, false);
      c.done();
    }

    void Status::set_total_stored_data_compressed(size_t size)
    {
      _total_stored_data_compressed = size;

      Connection c;
      mongo::BSONObj query = BSON("_id" << _processing_id);
      mongo::BSONObj update_value = BSON("$set" << BSON("total_stored_data_compressed" << (long long) _total_stored_data_compressed.load()));
      c->update(dba::helpers::collection_name(dba::Collections::PROCESSING()), query, update_value, false, false);
      c.done();
    }

    size_t Status::regions()
    {
      return _total_regions;
    }

    size_t Status::size()
    {
      return _total_size;
    }

    typedef std::shared_ptr<Status> StatusPtr;

    StatusPtr build_status(const std::string& _id)
    {
      return std::shared_ptr<Status>(new Status(_id));
    }

    StatusPtr build_dummy_status()
    {
      return std::shared_ptr<Status>(new Status(DUMMY_REQUEST));
    }

    std::string DUMMY_REQUEST = "dummy";
    std::map<OP, std::string> OP_names = create_OP_names_map();
  }
}

