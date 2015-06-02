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
#include <stack>
#include <string>

#include "../extras/utils.hpp"

namespace epidb {

  class StringBuilder;

  namespace processing {

    enum OP {
      A
    };

    struct OperationStatus {
      OP op;
      time_t start;
      time_t end;
    };

    class Status {
      std::string request_id;

      std::stack<OperationStatus> operations;

      std::atomic_size_t total_regions;
      std::atomic_size_t total_size;


    public:
      Status(const std::string &id);
      void start_operation(OP op);
      void end_operation();
      void sum_regions(size_t qtd);
      void sum_size(size_t size);
    };

    typedef std::shared_ptr<Status> StatusPtr;

    StatusPtr build_status(const std::string& _id);
    StatusPtr build_dummy_status();

    bool count_regions(const std::string &query_id, const std::string &user_key, processing::StatusPtr status, size_t &count, std::string &msg);

    bool get_regions(const std::string &query_id, const std::string &format, const std::string &user_key, processing::StatusPtr status, StringBuilder &sb, std::string &msg);

    bool score_matrix(const std::vector<std::pair<std::string, std::string>> &experiments_formats, const std::string &aggregation_function, const std::string &regions_query_id, const std::string &user_key, processing::StatusPtr status, std::string &matrix, std::string &msg);

    bool get_experiments_by_query(const std::string &query_id, const std::string &user_key, processing::StatusPtr status, std::vector<utils::IdName>& experiments, std::string &msg);
  }
}

#endif
