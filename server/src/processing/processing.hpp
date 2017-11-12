//
//  processing.hpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 28.01.15.
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

#ifndef EPIDB_PROCESSING_PROCESSING_HPP
#define EPIDB_PROCESSING_PROCESSING_HPP

#include <atomic>
#include <chrono>
#include <memory>
#include <string>
#include <vector>

#include "../datatypes/regions.hpp"
#include "../datatypes/user.hpp"

#include "../extras/utils.hpp"

#include "../errors.hpp"

namespace epidb {

#define INIT_PROCESSING(_WHAT, _STATUS) {                  \
    IS_PROCESSING_CANCELLED(_STATUS)                         \
    status->start_operation(_WHAT);                          \
  }                                                          \

#define IS_PROCESSING_CANCELLED(_STATUS) {                 \
    bool is_canceled = false;                                \
    if (!_STATUS->is_canceled(is_canceled, msg)) {           \
      return false;                                          \
    }                                                        \
    if (is_canceled) {                                       \
      msg = Error::m(ERR_REQUEST_CANCELED);                  \
      return false;                                          \
    }                                                        \
  }                                                          \

  class StringBuilder;

  namespace processing {

    extern std::string DUMMY_REQUEST;

    enum OP {
      PROCESS_QUERY,
      GET_EXPERIMENT_BY_QUERY,
      COUNT_REGIONS,
      RETRIEVE_EXPERIMENT_SELECT_QUERY,
      RETRIEVE_ANNOTATION_SELECT_QUERY,
      RETRIEVE_INTERSECTION_QUERY,
      RETRIEVE_MERGE_QUERY,
      RETRIEVE_FILTER_QUERY,
      RETRIEVE_TILING_QUERY,
      RETRIEVE_QUERY_REGION_SET,
      RETRIEVE_GENES_DATA,
      RETRIEVE_FLANK_QUERY,
      RETRIEVE_EXPRESSIONS_DATA,
      RETRIEVE_OVERLAP_QUERY,
      RETRIEVE_FIND_MOTIF_QUERY,
      PROCESS_AGGREGATE,
      PROCESS_DISTINCT,
      PROCESS_BINNING,
      PROCESS_COUNT,
      PROCESS_COVERAGE,
      PROCESS_GET_EXPERIMENTS_BY_QUERY,
      PROCESS_GET_REGIONS,
      PROCESS_SCORE_MATRIX,
      PROCESS_CALCULATE_GO_ENRICHMENT,
      PROCESS_ENRICH_REGIONS_OVERLAP,

      PROCESS_ENRICH_REGIONS_FAST,
      PROCESS_ENRICH_REGIONS_FAST_COMPARE_TO,
      PROCESS_ENRICH_REGIONS_FAST_STORE_BITMAP,
      PROCESS_ENRICH_REGIONS_FAST_LOAD_BITMAP,
      PROCESS_ENRICH_REGIONS_FAST_BUILD_BITMAP,
      PROCESS_ENRICH_REGIONS_FAST_GET_BITMAP_REGIONS,
      PROCESS_ENRICH_REGIONS_FAST_BITMAP_QUERY,
      PROCESS_ENRICH_REGIONS_FAST_BITMAP_EXPERIMENT,

      FORMAT_OUTPUT,
      BUILDING_OUTPUT,
      COMPRESSING_OUTPUT
    };

    extern std::map<OP, std::string> OP_names;

    std::string& op_name(const OP& op);

    class RunningOp {
      const mongo::OID _id;
      const std::string &_processing_id;
      const OP _op;
      const mongo::BSONObj params;
      const boost::posix_time::ptime _start_time;
      size_t _steps;
      size_t _actual_step;

    public:
      RunningOp(const std::string& processing_id, const OP& op, const mongo::BSONObj& params);
      ~RunningOp();

      void set_total_steps(size_t steps);
      void increment_step();
    };

    class RunningCache;

    class Status {
      std::string _request_id;
      std::string _processing_id;

      const long long _maximum_memory;

      bool _canceled;

      std::atomic_llong _total_regions;
      std::atomic_llong _total_size;
      std::atomic_llong _total_stored_data;
      std::atomic_llong _total_stored_data_compressed;
      std::chrono::seconds _last_update;
      const std::chrono::seconds _update_time_out;

      std::unique_ptr<RunningCache> _running_cache;

      mongo::BSONObj toBson();
      void update_values_in_db();

    public:
      Status(const std::string &request_id, const long long maximum_memory);
      ~Status();
      RunningOp start_operation(OP op, const mongo::BSONObj& params = mongo::BSONObj());
      void sum_regions(const long long qtd);
      void subtract_regions(const long long qtd);
      long long sum_size(const long long size);
      long long subtract_size(const long long qtd);
      void set_total_stored_data(long long size);
      void set_total_stored_data_compressed(long long size);
      long long total_regions();
      long long total_size();
      long long maximum_size();
      bool is_allowed_size(size_t size);
      bool is_canceled(bool& ret, std::string& msg);
      std::unique_ptr<RunningCache>& running_cache();
    };

    typedef std::shared_ptr<Status> StatusPtr;

    struct CoverageInfo {
      const std::string chromosome_name;
      size_t chromosome_size;
      size_t total;
    };

    StatusPtr build_status(const std::string& _id, const long long maximum_memory);
    StatusPtr build_dummy_status();

    bool count_regions(const datatypes::User& user,
                       const std::string &query_id,
                       processing::StatusPtr status, size_t &count, std::string &msg);

    bool binning(const datatypes::User& user,
                 const std::string& query_id, const std::string& column_name, const int bars,
                 const processing::StatusPtr status, mongo::BSONObj& counts, std::string& msg);

    bool distinct(const datatypes::User& user,
                  const std::string& query_id, const std::string& column_name,
                  const processing::StatusPtr status, mongo::BSONObj& counts, std::string& msg);

    bool calculate_enrichment(const datatypes::User& user,
                              const std::string& query_id, const std::string& gene_model,
                              processing::StatusPtr status, mongo::BSONObj& result, std::string& msg);

    bool coverage(const datatypes::User& user,
                  const std::string &query_id, const std::string &genome,
                  processing::StatusPtr status, std::vector<CoverageInfo> &coverage_infos, std::string &msg);

    bool get_regions(const datatypes::User& user,
                     const std::string &query_id, const std::string &format,
                     processing::StatusPtr status, StringBuilder &sb, std::string &msg);

    bool score_matrix(const datatypes::User& user,
                      const std::vector<std::pair<std::string, std::string>> &experiments_formats, const std::string &aggregation_function,
                      const std::string &regions_query_id,
                      processing::StatusPtr status, std::string &matrix, std::string &msg);


    bool enrich_regions_fast(const datatypes::User& user, const std::string& query_id, const std::vector<utils::IdName>& names,
                             processing::StatusPtr status,
                             mongo::BSONObj& result, std::string& msg);

    bool lola(const datatypes::User& user,
              const std::string& query_id, const std::string& universe_query_id,
              const mongo::BSONObj& datasets, const std::string& genome,
              processing::StatusPtr status, mongo::BSONObj& result, std::string& msg);

    bool get_experiments_by_query(const datatypes::User& user,
                                  const std::string &query_id,
                                  processing::StatusPtr status, std::vector<utils::IdName>& experiments, std::string &msg);

    bool format_regions(const std::string &output_format, ChromosomeRegionsList &chromosomeRegionsList, processing::StatusPtr status, StringBuilder &sb, std::string &msg);

    bool is_canceled(processing::StatusPtr status, std::string& msg);
  }
}

#endif
