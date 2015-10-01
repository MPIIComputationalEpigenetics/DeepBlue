//
//  processing.hpp
//  epidb
//
//  Created by Felipe Albrecht on 28.01.15.
//  Copyright (c) 2015 Max Planck Institute for Computer Science. All rights reserved.
//

#ifndef EPIDB_PROCESSING_PROCESSING_HPP
#define EPIDB_PROCESSING_PROCESSING_HPP

#include <atomic>
#include <memory>
#include <string>
#include <vector>

#include "../extras/utils.hpp"

namespace epidb {

  class StringBuilder;

  namespace processing {

    extern std::string DUMMY_REQUEST;

    enum OP {
      GET_EXPERIMENT_BY_QUERY = 10,
      COUNT_REGIONS = 11,
      RETRIEVE_EXPERIMENT_SELECT_QUERY = 30,
      RETRIEVE_ANNOTATION_SELECT_QUERY = 31,
      RETRIEVE_INTERSECTION_QUERY = 32,
      RETRIEVE_MERGE_QUERY = 33,
      RETRIEVE_FILTER_QUERY = 34,
      RETRIEVE_TILING_QUERY = 35,
      RETRIEVE_QUERY_REGION_SET = 36,
      RETRIEVE_GENES_DATA = 37,
      PROCESS_AGGREGATE = 50
    };

    extern std::map<OP, std::string> OP_names;

    std::string& op_name(const OP& op);

    class RunningOp {
      const mongo::OID _id;
      const std::string &_processing_id;
      const OP _op;
      const mongo::BSONObj params;
      const boost::posix_time::ptime _start_time;

    public:
      RunningOp(const std::string& processing_id, const OP& op, const mongo::BSONObj& params);
      ~RunningOp();
    };

    class Status {
      std::string _request_id;
      std::string _processing_id;

      const long long _maximum_memory;

      bool _canceled;

      std::atomic_llong _total_regions;
      std::atomic_llong _total_size;
      std::atomic_llong _total_stored_data;
      std::atomic_llong _total_stored_data_compressed;

      mongo::BSONObj toBson();

    public:
      Status(const std::string &request_id, const long long maximum_memory);
      ~Status();
      RunningOp start_operation(OP op, const mongo::BSONObj& params = mongo::BSONObj());
      void sum_regions(const long long qtd);
      long long sum_size(const long long size);
      void set_total_stored_data(long long size);
      void set_total_stored_data_compressed(long long size);
      long long total_regions();
      long long total_size();
      long long maximum_size();
      bool is_canceled(bool& ret, std::string& msg);
    };

    typedef std::shared_ptr<Status> StatusPtr;

    StatusPtr build_status(const std::string& _id, const long long maximum_memory);
    StatusPtr build_dummy_status();

    bool count_regions(const std::string &query_id, const std::string &user_key, processing::StatusPtr status, size_t &count, std::string &msg);

    bool get_regions(const std::string &query_id, const std::string &format, const std::string &user_key, processing::StatusPtr status, StringBuilder &sb, std::string &msg);

    bool score_matrix(const std::vector<std::pair<std::string, std::string>> &experiments_formats, const std::string &aggregation_function, const std::string &regions_query_id, const std::string &user_key, processing::StatusPtr status, std::string &matrix, std::string &msg);

    bool get_experiments_by_query(const std::string &query_id, const std::string &user_key, processing::StatusPtr status, std::vector<utils::IdName>& experiments, std::string &msg);
  }
}

#endif
