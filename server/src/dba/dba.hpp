
//
//  dba.hpp
//  epidb
//
//  Created by Felipe Albrecht on 01.06.14.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#ifndef EPIDB_DBA_DBA_HPP
#define EPIDB_DBA_DBA_HPP

#include <string>
#include <vector>

#include <mongo/bson/bson.h>

#include "../regions.hpp"

#include "../engine/commands.hpp"
#include "../parser/field_type.hpp"
#include "../parser/genome_data.hpp"
#include "../parser/parser_factory.hpp"

namespace epidb {
  namespace dba {

    bool get_genome_by_region_set(const long long region_set_id, std::string  &genome, std::string &msg);

    bool get_source_name_by_region_set(const long long region_set_id, const std::string &def,
                                       std::string &experiment_name, std::string &msg);

    bool init_system(const std::string &name, const std::string &email, const std::string &institution,
                     const std::string &key, std::string &msg);

    bool add_user(const std::string &name, const std::string &email, const std::string &institution,
                  const std::string &key,
                  std::string &user_id, std::string &msg);

    bool add_genome(const std::string &name, const std::string &norm_name,
                    const std::string &description, const std::string &norm_description,
                    const parser::ChromosomesInfo &g,
                    const std::string &user_key, const std::string &ip,
                    std::string &genome_id, std::string &msg);

    bool add_chromosome_sequence(const std::string &genome, const std::string &norm_genome,
                                 const std::string &chromosome,
                                 const std::string &sequence,
                                 const std::string &user_key, std::string &msg);

    bool add_epigenetic_mark(const std::string &name, const std::string &norm_name,
                             const std::string &description, const std::string &norm_description,
                             const std::string &user_key,
                             std::string &epigenetic_mark_id, std::string &msg);

    bool add_biosource(const std::string &name, const std::string &norm_name,
                        const std::string &description, const std::string &norm_description,
                        const Metadata &extra_metadata,
                        const std::string &user_key,
                        std::string &biosource_id, std::string &msg);

    bool add_sample(const std::string &biosource_name, const std::string &norm_biosource_name,
                    const Metadata &metadata,
                    const std::string &user_key,
                    std::string &sample_id, std::string &msg);

    bool add_sample_field(const std::string &name, const std::string &norm_name,
                          const std::string &type,
                          const std::string &description, const std::string &norm_description,
                          const std::string &user_key,
                          std::string &sample_field_id, std::string &msg);

    bool add_technique(const std::string &name, const std::string &norm_name,
                       const std::string &description, const std::string &norm_description,
                       const Metadata &extra_metadata,
                       const std::string &user_key,
                       std::string &biosource_id, std::string &msg);

    bool add_project(const std::string &name, const std::string &norm_name,
                     const std::string &description, const std::string &norm_description,
                     const std::string &user_key,
                     std::string &project_id, std::string &msg);

    bool set_biosource_synonym(const std::string &biosource_name, const std::string &synonymous,
                                bool is_biosource, const bool is_syn,
                                const std::string &user_key, std::string &msg);

    bool set_biosource_scope(const std::string &biosource_more_embracing, const std::string &norm_biosource_more_embracing,
                              const std::string &biosource_less_embracing, const std::string &norm_biosource_less_embracing,
                              bool more_embracing_is_syn, const bool less_embracing_is_syn,
                              const std::string &user_key, std::string &msg);


    /**
     * Validation
     */

    bool is_valid_biosource_name(const std::string &name, const std::string &norm_name, std::string &msg);

    bool is_valid_sample_field_name(const std::string &name, const std::string &norm_name, std::string &msg);

    bool is_valid_epigenetic_mark(const std::string &name, const std::string &norm_name, std::string &msg);

    bool is_project_valid(const std::string &name, const std::string &norm_name, std::string &msg);

    bool is_valid_genome(const std::string &genome, const std::string &norm_genome, std::string &msg);

    bool is_valid_email(const std::string &email, std::string &msg);

    bool is_initialized(bool &ret, std::string &msg);

    bool is_admin_key(const std::string &admin_key, bool &ret, std::string &msg);


    /**
     * Checks
     */

    bool check_user(const std::string &user_key, bool &r, std::string &msg);

    bool check_genome(const std::string &genome, bool &r, std::string &msg);

    bool check_epigenetic_mark(const std::string &epigenetic_mark, bool &r, std::string &msg);

    bool check_biosource(const std::string &biosource_name_norm, bool &r, std::string &msg);

    bool check_biosource_synonym(const std::string &biosource_syn_norm, bool &r, std::string &msg);

    bool check_sample(const std::string &biosource_name_norm, bool &r, std::string &msg);

    bool check_sample_field(const std::string &biosource_name_norm, bool &r, std::string &msg);

    bool check_technique(const std::string &norm_technique_name, bool &r, std::string &msg);

    bool check_project(const std::string &project, bool &r, std::string &msg);

    bool check_annotation(const std::string &norm_annotation, const std::string &genome, bool &ok, std::string &msg);

    bool check_query(const std::string &user_key, const std::string &query_id, bool &r, std::string &msg);

    bool check_experiment_name(const std::string &name, const std::string &norm_name, const std::string &user_key,
                               bool &ok, std::string &msg);


    /**
     * Getters
     */
    bool get_biosource_scope(const std::string &biosource_name, const std::string &norm_biosource_name,
                              bool is_biosource, const std::string &user_key,
                              std::vector<utils::IdName> &biosources, std::string &msg);

    bool get_biosource_synonyms(const std::string &biosource_name, const std::string &norm_biosource_name,
                                 bool is_biosource, const std::string &user_key,
                                 std::vector<utils::IdName> &syns, std::string &msg);

    bool get_user_name(const std::string &user_key, std::string &name, std::string &msg);

    bool get_user_name(const std::string &user_key, utils::IdName &id_name, std::string &msg);

    bool count_experiments(unsigned long long &size, const std::string &user_key, std::string &msg);

    /**
     * Permissions
     */

    bool set_user_admin(const std::string &user_id, const bool value, std::string &msg);

    /**
     * Pattern
     **/
    bool process_pattern(const std::string &genome, const std::string &pattern, const bool overlap,
                         const std::string &user_key, const std::string &ip,
                         std::string &annotation_id, std::string &msg);

    bool find_annotation_pattern(const std::string &genome, const std::string &pattern, const bool overlap,
                                 DatasetId &dataset_id, std::string &msg);
  }
}

#endif
