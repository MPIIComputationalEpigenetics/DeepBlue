
//
//  dba.hpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 01.06.14.
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

#ifndef EPIDB_DBA_DBA_HPP
#define EPIDB_DBA_DBA_HPP

#include <string>
#include <vector>

#include <mongo/bson/bson.h>

#include "../datatypes/regions.hpp"
#include "../datatypes/metadata.hpp"
#include "../datatypes/user.hpp"

#include "../parser/genome_data.hpp"
#include "../parser/parser_factory.hpp"

namespace epidb {
  namespace dba {

    bool get_genome_by_region_set(const long long region_set_id, std::string  &genome, std::string &msg);

    bool get_source_name_by_region_set(const long long region_set_id, const std::string &def,
                                       std::string &experiment_name, std::string &msg);

    bool init_system(const std::string &name, const std::string &email, const std::string &institution,
                     datatypes::User& admin_user, std::string &msg);

    bool create_indexes(std::string &msg);

    bool add_genome(const datatypes::User& user,
                    const std::string &name, const std::string &norm_name,
                    const std::string &description, const std::string &norm_description,
                    const parser::ChromosomesInfo &g,
                    const std::string &ip,
                    std::string &genome_id, std::string &msg);

    bool add_chromosome_sequence(const datatypes::User& user,
                                 const std::string &genome, const std::string &norm_genome,
                                 const std::string &chromosome,
                                 const std::string &sequence,
                                 std::string &msg);

    bool add_epigenetic_mark(const datatypes::User& user,
                             const std::string &name, const std::string &norm_name,
                             const std::string &description, const std::string &norm_description,
                             const datatypes::Metadata &extra_metadata,
                             std::string &epigenetic_mark_id, std::string &msg);

    bool add_biosource(const datatypes::User& user,
                       const std::string &name, const std::string &norm_name,
                       const std::string &description, const std::string &norm_description,
                       const datatypes::Metadata &extra_metadata,
                       std::string &biosource_id, std::string &msg);

    bool add_sample(const datatypes::User& user,
                    const std::string &biosource_name, const std::string &norm_biosource_name,
                    const datatypes::Metadata &metadata,
                    std::string &sample_id, std::string &msg);

    bool add_technique(const datatypes::User& user,
                       const std::string &name, const std::string &norm_name,
                       const std::string &description, const std::string &norm_description,
                       const datatypes::Metadata &extra_metadata,
                       std::string &biosource_id, std::string &msg);
    /**
     * Validation
     */

    bool is_valid_biosource_name(const std::string &name, const std::string &norm_name, std::string &msg);

    bool is_valid_epigenetic_mark(const std::string &name, const std::string &norm_name, std::string &msg);

    bool is_valid_technique_name(const std::string &name, const std::string &norm_name, std::string &msg);

    bool is_project_valid(const std::string &name, const std::string &norm_name, std::string &msg);

    bool is_valid_genome(const std::string &genome, const std::string &norm_genome, std::string &msg);

    bool is_initialized();

    /**
     * Getters
     */
    bool get_biosource_children(const std::string &biosource_name, const std::string &norm_biosource_name,
                                bool is_biosource,
                                std::vector<utils::IdName> &biosources, std::string &msg);


    bool get_biosource_parents(const std::string &biosource_name, const std::string &norm_biosource_name,
                               bool is_biosource,
                               std::vector<utils::IdName> &related_biosources, std::string &msg);

    bool get_biosource_synonyms(const std::string &biosource_name, const std::string &norm_biosource_name,
                                bool is_biosource,
                                std::vector<utils::IdName> &syns, std::string &msg);

    bool collection_size(const std::string collection, mongo::BSONObj query, size_t& count, std::string& msg);

    /**
     * Pattern
     **/
    bool process_pattern(const std::string &genome, const std::string &motif, const bool overlap,
                         std::vector<std::string> &chromosomes, const long long start, const long long end,
                         ChromosomeRegionsList& pattern_regions, std::string &msg);
  }
}

#endif
