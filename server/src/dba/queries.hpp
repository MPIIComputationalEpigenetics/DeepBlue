//
//  regions.h
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 09.07.13.
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

#ifndef EPIDB_DBA_QUERIES_HPP
#define EPIDB_DBA_QUERIES_HPP

#include <iostream>
#include <map>

#include <mongo/bson/bson.h>

#include "dba.hpp"

#include "../datatypes/regions.hpp"
#include "../datatypes/user.hpp"

#include "../extras/utils.hpp"

namespace epidb {
  namespace dba {
    namespace query {

      void invalidate_cache();

      bool store_query(const datatypes::User& user,
                       const std::string &type,
                       const mongo::BSONObj &args,
                       std::string &query_id, std::string &msg);

      bool modify_query(const datatypes::User& user,
                        const std::string &query_id, const std::string &key, const std::string &value,
                        std::string &new_query_id, std::string &msg);

      bool retrieve_query(const datatypes::User& user,
                          const std::string &query_id,
                          processing::StatusPtr status, ChromosomeRegionsList &regions, std::string &msg,
                          bool reduced_mode = false);

      bool get_experiments_by_query(const datatypes::User& user,
                                    const std::string &query_id,
                                    processing::StatusPtr status, std::vector<utils::IdName> &experiments_name, std::string &msg);

      bool retrieve_experiment_select_query(const datatypes::User& user,
                                            const mongo::BSONObj &query,
                                            processing::StatusPtr status, ChromosomeRegionsList &regions, std::string &msg,
                                            bool reduced_mode = false);

      bool count_regions(const datatypes::User& user,
                         const std::string &query_id,
                         processing::StatusPtr status, size_t &count, std::string &msg);

      bool build_annotation_query(const datatypes::User& user,
                                  const mongo::BSONObj &query,
                                  mongo::BSONObj &regions_query, std::vector<std::string> &annotations_id,
                                  std::string &msg);


      bool build_experiment_query(const datatypes::User& user,
                                  const mongo::BSONObj &args,
                                  mongo::BSONObj &regions_query, std::string &msg);

      bool build_experiment_query(const int start, const int end, const std::string &experiment_name,
                                  mongo::BSONObj &regions_query, std::string &msg);

      bool build_experiment_query(const int start, const int end, const mongo::BSONArray& datasets_array,
                                  mongo::BSONObj &regions_query, std::string &msg);


      bool retrieve_annotation_select_query(const datatypes::User& user,
                                            const mongo::BSONObj &query,
                                            processing::StatusPtr status, ChromosomeRegionsList &regions, std::string &msg);

      bool retrieve_genes_select_query(const datatypes::User& user,
                                       const mongo::BSONObj &query,
                                       processing::StatusPtr status, ChromosomeRegionsList &regions, std::string &msg);

      bool retrieve_find_motif_query(const datatypes::User& user,
                                     const mongo::BSONObj &query,
                                     processing::StatusPtr status, ChromosomeRegionsList &regions, std::string &msg);

      bool retrieve_expression_select_query(const datatypes::User& user,
                                            const mongo::BSONObj &query,
                                            processing::StatusPtr status, ChromosomeRegionsList &regions, std::string &msg);

      bool retrieve_intersection_query(const datatypes::User& user,
                                       const mongo::BSONObj &query,
                                       processing::StatusPtr status, ChromosomeRegionsList &regions, std::string &msg);

      bool retrieve_overlap_query(const datatypes::User& user,
                                  const mongo::BSONObj &query,
                                  processing::StatusPtr status, ChromosomeRegionsList &regions, std::string &msg);

      bool retrieve_flank_query(const datatypes::User& user,
                                const mongo::BSONObj &query,
                                processing::StatusPtr status, ChromosomeRegionsList &regions, std::string &msg);

      bool retrieve_extend_query(const datatypes::User& user,
                                 const mongo::BSONObj &query,
                                 processing::StatusPtr status, ChromosomeRegionsList &regions, std::string &msg);

      bool retrieve_merge_query(const datatypes::User& user,
                                const mongo::BSONObj &query,
                                processing::StatusPtr status, ChromosomeRegionsList &regions, std::string &msg);

      bool retrieve_query_region_set(const mongo::BSONObj &query,
                                     processing::StatusPtr status, ChromosomeRegionsList &regions, std::string &msg);

      bool retrieve_annotation_select_query(const datatypes::User& user,
                                            const mongo::BSONObj &query,
                                            processing::StatusPtr status, ChromosomeRegionsList &regions, std::string &msg);

      bool retrieve_filter_query(const datatypes::User& user,
                                 const mongo::BSONObj &query,
                                 processing::StatusPtr status, ChromosomeRegionsList &regions, std::string &msg);

      bool retrieve_filter_by_motif_query(const datatypes::User& user,
                                          const mongo::BSONObj & query,
                                          processing::StatusPtr status, ChromosomeRegionsList & filtered_regions, std::string & msg);

      bool add_tiling(const std::string &genome, const size_t &tiling_size,
                      std::string &tiling_id, std::string &msg);

      bool retrieve_tiling_query(const mongo::BSONObj &query,
                                 processing::StatusPtr status, ChromosomeRegionsList &regions, std::string &msg);

      bool process_aggregate(const datatypes::User& user,
                             const mongo::BSONObj &query,
                             processing::StatusPtr status, ChromosomeRegionsList &regions, std::string &msg);

      bool is_canceled(processing::StatusPtr status, std::string& msg);

      bool get_main_experiment_data(const datatypes::User& user, const std::string &query_id, const std::string field_key,
                                    processing::StatusPtr status, std::vector<std::string>& values, std::string &msg);

      /* These two functions must not be accessed directly. Use the cache:: methods */
      /* TODO: move to another file */
      bool __get_columns_from_dataset(const DatasetId & dataset_id, std::vector<mongo::BSONObj> &columns, std::string & msg);

      bool __get_bson_by_dataset_id(DatasetId dataset_id, mongo::BSONObj &obj, std::string &msg);
    }
  }
}

#endif
