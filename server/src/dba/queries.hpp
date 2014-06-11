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

#include "../regions.hpp"

#include "../extras/utils.hpp"

namespace epidb {
  namespace dba {
    namespace query {

      bool store_query(const std::string &type,
                       const mongo::BSONObj &args, const std::string &user_key,
                       std::string &query_id, std::string &msg);

      bool retrieve_query(const std::string &user_key, const std::string &query_id,
                          ChromosomeRegionsList &regions, std::string &msg);

      bool get_experiments_by_query(const std::string &user_key, const std::string &query_id,
                                    std::vector<utils::IdName> &experiments_name, std::string &msg);

      bool retrieve_experiment_select_query(const std::string &user_key, const mongo::BSONObj &query,
                                            ChromosomeRegionsList &regions, std::string &msg);

      bool count_regions(const std::string &user_key, const std::string &query_id,
                         size_t &count, std::string &msg);

      const mongo::BSONObj build_query(const mongo::BSONObj &args);

      bool build_annotation_query(const std::string &user_key, const mongo::BSONObj &query,
                                  mongo::BSONObj &regions_query, std::vector<std::string> &annotations_id,
                                  std::string &msg);

      bool build_experiment_query(const std::string &user_key, const mongo::BSONObj &query,
                                  mongo::BSONObj &regions_query, std::vector<std::string> &experiments_id,
                                  std::string &msg);

      bool retrieve_annotation_select_query(const std::string &user_key, const mongo::BSONObj &query,
                                            ChromosomeRegionsList &regions, std::string &msg);

      bool retrieve_intersection_query(const std::string &user_key, const mongo::BSONObj &query,
                                       ChromosomeRegionsList &regions, std::string &msg);

      bool retrieve_merge_query(const std::string &user_key, const mongo::BSONObj &query,
                                ChromosomeRegionsList &regions, std::string &msg);

      bool retrieve_annotation_select_query(const std::string &user_key, const mongo::BSONObj &query,
                                            ChromosomeRegionsList &regions, std::string &msg);

      bool retrieve_filter_query(const std::string &user_key, const mongo::BSONObj &query,
                                 ChromosomeRegionsList &regions, std::string &msg);

      bool add_tiling(const std::string &genome, const size_t &tiling_size,
                      std::string &tiling_id, std::string &msg);

      bool retrieve_tiling_query(const mongo::BSONObj &query,
                                 ChromosomeRegionsList &regions, std::string &msg);

      bool process_aggregate(const std::string &user_key, const mongo::BSONObj &query,
                             ChromosomeRegionsList &regions, std::string &msg);
    }
  }
}

#endif
