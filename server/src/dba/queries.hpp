//
//  regions.h
//  epidb
//
//  Created by Felipe Albrecht on 09.07.13.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#ifndef EPIDB_DBA_QUERIES_HPP
#define EPIDB_DBA_QUERIES_HPP

#include <iostream>
#include <map>

#include <mongo/bson/bson.h>

#include "dba.hpp"

#include "../datatypes/regions.hpp"

#include "../extras/utils.hpp"

namespace epidb {
  namespace dba {
    namespace query {

      void invalidate_cache();

      bool store_query(const std::string &type,
                       const mongo::BSONObj &args, const std::string &user_key,
                       std::string &query_id, std::string &msg);

      bool retrieve_query(const std::string &user_key, const std::string &query_id,
                          processing::StatusPtr status, ChromosomeRegionsList &regions, std::string &msg);

      bool get_experiments_by_query(const std::string &user_key, const std::string &query_id,
                                    processing::StatusPtr status, std::vector<utils::IdName> &experiments_name, std::string &msg);

      bool retrieve_experiment_select_query(const mongo::BSONObj &query,
                                            processing::StatusPtr status, ChromosomeRegionsList &regions, std::string &msg);

      bool count_regions(const std::string &query_id, const std::string &user_key,
                         processing::StatusPtr status, size_t &count, std::string &msg);

      const mongo::BSONObj build_query(const mongo::BSONObj &args);

      bool build_annotation_query(const std::string &user_key, const mongo::BSONObj &query,
                                  mongo::BSONObj &regions_query, std::vector<std::string> &annotations_id,
                                  std::string &msg);

      bool build_experiment_query(const int start, const int end, const std::string &experiment_name,
                                  mongo::BSONObj &regions_query, std::string &msg);

      bool build_experiment_query(const int start, const int end, const mongo::BSONArray& datasets_array,
                                  mongo::BSONObj &regions_query, std::string &msg);

      bool build_experiment_query(const int start, const int end, const mongo::BSONArray& datasets_array,
                                  mongo::BSONObj &regions_query, std::string &msg);

      bool build_experiment_query(const mongo::BSONObj &query,
                                  mongo::BSONObj &regions_query, std::vector<std::string> &experiments_id,
                                  std::string &msg);

      bool retrieve_annotation_select_query(const std::string &user_key, const mongo::BSONObj &query,
                                            processing::StatusPtr status, ChromosomeRegionsList &regions, std::string &msg);

      bool retrieve_intersection_query(const std::string &user_key, const mongo::BSONObj &query,
                                       processing::StatusPtr status, ChromosomeRegionsList &regions, std::string &msg);

      bool retrieve_merge_query(const std::string &user_key, const mongo::BSONObj &query,
                                processing::StatusPtr status, ChromosomeRegionsList &regions, std::string &msg);

      bool retrieve_query_region_set(const mongo::BSONObj &query,
                                     processing::StatusPtr status, ChromosomeRegionsList &regions, std::string &msg);

      bool retrieve_annotation_select_query(const std::string &user_key, const mongo::BSONObj &query,
                                            processing::StatusPtr status, ChromosomeRegionsList &regions, std::string &msg);

      bool retrieve_filter_query(const std::string &user_key, const mongo::BSONObj &query,
                                 processing::StatusPtr status, ChromosomeRegionsList &regions, std::string &msg);

      bool add_tiling(const std::string &genome, const size_t &tiling_size,
                      std::string &tiling_id, std::string &msg);

      bool retrieve_tiling_query(const mongo::BSONObj &query,
                                 processing::StatusPtr status, ChromosomeRegionsList &regions, std::string &msg);

      bool process_aggregate(const std::string &user_key, const mongo::BSONObj &query,
                             processing::StatusPtr status, ChromosomeRegionsList &regions, std::string &msg);

      bool get_columns_from_dataset(const DatasetId &dataset_id, std::vector<mongo::BSONObj> &columns, std::string &msg);

      bool is_canceled(processing::StatusPtr status, std::string msg);
    }
  }
}

#endif
