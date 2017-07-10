//
//  retrieve.hpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 01.06.13.
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

#ifndef EPIDB_DBA_RETRIEVE_HPP
#define EPIDB_DBA_RETRIEVE_HPP

#include <map>
#include <string>
#include <vector>

#include <mongo/bson/bson.h>

#include "../datatypes/regions.hpp"
#include "../processing/processing.hpp"


namespace epidb {
  namespace dba {
    namespace retrieve {

      bool get_regions(const std::string &genome, const std::string &chromosome,
                       const mongo::BSONObj &regions_query, const bool full_overlap,
                       processing::StatusPtr status,
                       Regions &regions, std::string &msg,
                       bool reduced_mode = false);

      bool get_regions(const std::string &genome, const std::vector<std::string> &chromosomes,
                       const mongo::BSONObj &regions_query, const bool full_overlap,
                       processing::StatusPtr status,
                       ChromosomeRegionsList &results, std::string &msg,
                       bool reduced_mode = false);

      bool count_regions(const std::string &genome, const std::string &chromosome,
                         const mongo::BSONObj &regions_query, const bool full_overlap,
                         processing::StatusPtr status,
                         size_t &count);

      bool count_regions(const std::string &genome,
                         std::vector<std::string> &chromosomes,
                         const mongo::BSONObj &regions_query, const bool full_overlap,
                         processing::StatusPtr status,
                         size_t &size, std::string &msg);

      bool get_regions_preview(const std::string &genome, const std::string &chromosome, const mongo::BSONObj &regions_query,
                               Regions& regions, std::string &msg);

    }
  }
}

#endif
