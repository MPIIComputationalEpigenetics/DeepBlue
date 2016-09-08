//
//  info.hpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 04.04.14.
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

#ifndef EPIDB_DBA_INFO_HPP
#define EPIDB_DBA_INFO_HPP

#include "../datatypes/metadata.hpp"

#include <string>
#include <map>

namespace epidb {
  namespace dba {
    namespace info {

      bool get_genome(const std::string &id, datatypes::Metadata &res, mongo::BSONObj& chromosomes, std::string &msg, bool full = false);

      bool get_project(const std::string &id, const std::vector<std::string>& user_projects,
                       datatypes::Metadata &res, std::string &msg, bool full = false);

      bool get_biosource(const std::string &id,
                         std::map<std::string, std::string> &metadata,
                         std::map<std::string, std::string> &extra_metadata,
                         std::vector<std::string> &synonyms,
                         std::string &msg,
                         bool full = false);

      bool get_technique(const std::string &id, std::map<std::string, std::string> &res,
                         std::map<std::string, std::string> &metadata, std::string &msg, bool full = false);

      bool get_sample_by_id(const std::string &id, std::map<std::string, std::string> &, std::string &msg, bool full = false);

      bool get_epigenetic_mark(const std::string &id, std::map<std::string, std::string> &,
                               std::map<std::string, std::string> &metadata,
                               std::string &msg, bool full = false);

      bool get_annotation(const std::string &id,
                          std::map<std::string, std::string> &metadata,
                          std::map<std::string, std::string> &extra_metadata,
                          std::vector<std::map<std::string, std::string> > &columns,
                          std::map<std::string, std::string> &upload_info,
                          std::string &msg, bool full = false);

      bool get_experiment(const std::string &id, const std::vector<std::string>& user_projects,
                          std::map<std::string, std::string> &metadata,
                          std::map<std::string, std::string> &extra_metadata,
                          std::map<std::string, std::string> &sample_info,
                          std::vector<std::map<std::string, std::string> > &columns,
                          std::map<std::string, std::string> &upload_info,
                          std::string &msg, bool full = false);

      bool get_experiment_set_info(const std::string& id,  mongo::BSONObj& obj_metadata, std::string& msg);


      bool get_query(const std::string &id, std::map<std::string, std::string> &, std::string &msg);

      bool get_tiling_region(const std::string &id, std::map<std::string, std::string> &res, std::string &msg, bool full = false);

      bool get_column_type(const std::string &id, std::map<std::string, std::string> &res, std::string &msg);

      /*
      * \brief  Convert a map with an "name"-field containing a user-ID to one
      *         containing a user-name in that field
      * \param  map Map to be modified
      */
      bool id_to_name(std::map<std::string, std::string> &map, std::string &msg);
    }
  }
}

#endif