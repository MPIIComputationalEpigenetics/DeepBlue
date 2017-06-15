//
//  queue_processer.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 23.01.15.
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

#include <boost/asio.hpp>

#include "../mdbq/client.hpp"
#include "../processing/processing.hpp"

namespace mongo {
  class BSONObj;
}

namespace epidb {
  namespace engine {
    void queue_processer_run(size_t num);
    class QueueHandler : public mdbq::Client {
    public:
      boost::asio::io_service ios;
      size_t _id;
      QueueHandler(size_t id, std::string &url, std::string &prefix);
      void handle_task(const std::string& _id, const mongo::BSONObj &o);
      void run();

      bool process(const mongo::BSONObj &job, processing::StatusPtr status, mongo::BSONObj& result);
      bool process_count(const std::string &request_id, const std::string &user_key, processing::StatusPtr status, mongo::BSONObj& result);
      bool process_binning(const std::string &query_id, const std::string& column_name, const int bars, const std::string &user_key, processing::StatusPtr status, mongo::BSONObj& result);
      bool process_distinct(const std::string &query_id, const std::string& column_name, const std::string &user_key, processing::StatusPtr status, mongo::BSONObj& result);
      bool process_calculate_enrichment(const std::string &query_id, const std::string& gene_model, const std::string &user_key, processing::StatusPtr status, mongo::BSONObj& result);
      bool process_coverage(const std::string &request_id, const std::string & genome, const std::string &user_key, processing::StatusPtr status, mongo::BSONObj& result);
      bool process_get_regions(const std::string &query_id, const std::string &format, const std::string &user_key, processing::StatusPtr status, mongo::BSONObj& result);
      bool process_score_matrix(const mongo::BSONObj &experiments_formats, const std::string &aggregation_function, const std::string &regions_query_id, const std::string &user_key, processing::StatusPtr status, mongo::BSONObj& result);
      bool process_get_experiments_by_query(const std::string &query_id, const std::string &user_key, processing::StatusPtr status, mongo::BSONObj& result);
      bool is_canceled(processing::StatusPtr status, std::string msg);
    };
  }
}