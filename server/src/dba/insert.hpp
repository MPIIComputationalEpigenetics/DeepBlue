//
//  insert.hpp
//  DeepBlue Epigenomic Data Server

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
//  Created by Felipe Albrecht on 30.04.14.
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

#include <map>
#include <string>
#include <vector>

#include "../datatypes/metadata.hpp"
#include "../datatypes/regions.hpp"

#include "../parser/parser_factory.hpp"
#include "../parser/wig.hpp"

namespace epidb {
  namespace dba {

    /**
     * TODO: Unification of the insert_annotations in only 1 function that receives std::vector<Region>
     * TODO: Unification of the insert_experiment in only 1 function that receives std::vector<Region>
     */

    bool insert_annotation(const std::string &name, const std::string &norm_name,
                           const std::string &genome, const std::string &norm_genome,
                           const std::string &description, const std::string &norm_description,
                           const datatypes::Metadata &extra_metadata,
                           const std::string &user_key, const std::string &ip,
                           const parser::ChromosomeRegionsMap &map_regions,
                           const parser::FileFormat &format,
                           std::string &annotation_id, std::string &msg);

    bool insert_annotation(const std::string &name, const std::string &norm_name,
                           const std::string &genome, const std::string &norm_genome,
                           const std::string &description, const std::string &norm_description,
                           const datatypes::Metadata &extra_metadata,
                           const std::string &user_key, const std::string &ip,
                           const ChromosomeRegionsList &regions,
                           const parser::FileFormat &format,
                           std::string &annotation_id, std::string &msg);

    bool insert_annotation(const std::string &name, const std::string &norm_name,
                           const std::string &genome, const std::string &norm_genome,
                           const std::string &description, const std::string &norm_description,
                           const datatypes::Metadata &extra_metadata,
                           const std::string &user_key, const std::string &ip,
                           const parser::WigPtr &wig,
                           std::string &annotation_id, std::string &msg);

    bool insert_experiment(const std::string &name, const std::string &norm_name,
                           const std::string &genome, const std::string &norm_genome,
                           const std::string &epigenetic_mark, const std::string &norm_epigenetic_mark,
                           const std::string &sample_id, const std::string &technique, const std::string &norm_technique,
                           const std::string &project, const std::string &norm_project,
                           const std::string &description, const std::string &norm_description,
                           const datatypes::Metadata &extra_metadata,
                           const std::string &user_key, const std::string &ip,
                           const parser::WigPtr &wig,
                           std::string &experiment_id, std::string &msg);

    bool insert_experiment(const std::string &name, const std::string &norm_name,
                           const std::string &genome, const std::string &norm_genome,
                           const std::string &epigenetic_mark, const std::string &norm_epigenetic_mark,
                           const std::string &sample_id, const std::string &technique, const std::string &nrm_technique,
                           const std::string &project, const std::string &norm_project,
                           const std::string &description, const std::string &norm_description,
                           const datatypes::Metadata &extra_metadata,
                           const std::string &user_key, const std::string &ip,
                           const parser::ChromosomeRegionsMap &map_regions,
                           const parser::FileFormat &format,
                           std::string &experiment_id, std::string &msg);

    bool insert_query_region_set(const std::string &genome, const std::string &norm_genome,
                                 const std::string &user_key, const std::string &ip,
                                 const parser::ChromosomeRegionsMap &map_regions,
                                 const parser::FileFormat &format,
                                 int& dataset_id, std::string &msg);

  }
}