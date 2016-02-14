//
//  dba.cpp
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

#include <map>
#include <string>
#include <sstream>
#include <vector>

#include <time.h>
#include <math.h>

#include <mongo/bson/bson.h>

#include "../algorithms/patterns.hpp"

#include "../connection/connection.hpp"

#include "../datatypes/metadata.hpp"
#include "../datatypes/regions.hpp"
#include "../datatypes/user.hpp"

#include "../extras/utils.hpp"
#include "../parser/genome_data.hpp"
#include "../parser/parser_factory.hpp"
#include "../parser/wig_parser.hpp"

#include "../version.hpp"

#include "dba.hpp"
#include "users.hpp"
#include "config.hpp"
#include "collections.hpp"
#include "controlled_vocabulary.hpp"
#include "exists.hpp"
#include "full_text.hpp"
#include "genomes.hpp"
#include "helpers.hpp"
#include "insert.hpp"
#include "key_mapper.hpp"
#include "queries.hpp"
#include "retrieve.hpp"
#include "info.hpp"

#include "../errors.hpp"
#include "../log.hpp"

namespace epidb {
  namespace dba {

    bool is_initialized()
    {
      return helpers::check_exist(Collections::SETTINGS(), "initialized", true);
    }

    bool init_system(const std::string &name, const std::string &email, const std::string &institution,
                     std::string &user_key, std::string &msg)
    {
      Connection c;

      if (!create_indexes(msg)) {
        return false;
      }

      if (config::sharding()) {
        mongo::BSONObjBuilder builder;
        builder.append("enableSharding", config::DATABASE_NAME());
        mongo::BSONObj info;
        if (!c->runCommand("admin", builder.obj(), info)) {
          if (info["errmsg"].str() == "already enabled") {
            EPIDB_LOG_WARN("Sharding already enabled");
          } else {
            EPIDB_LOG_ERR("Error enableSharding: " << info.toString());
            msg = "Error Enable Sharding" + info.toString();
            return false;
          }
        } else {
          EPIDB_LOG("Shard enabled for: " << config::DATABASE_NAME() << " info: " << info.toString());
        }

        mongo::BSONObjBuilder movePrimaryBuilder;
        movePrimaryBuilder.append("movePrimary", config::DATABASE_NAME());
        movePrimaryBuilder.append("to", "shard0000");
        if (!c->runCommand("admin", movePrimaryBuilder.obj(), info)) {
          EPIDB_LOG_WARN("Error movePrimary: " << info.toString());
        } else {
          EPIDB_LOG("Primary moved: " << info.toString());
        }
      }

      datatypes::User user = datatypes::User(name, email, institution);
      user.generate_key();
      user.set_permission_level(datatypes::PermissionLevel::ADMIN);
      user_key = user.get_key();
      if (!dba::users::add_user(user, msg)) {
        return false;
      }

      mongo::BSONObjBuilder create_settings_builder;
      mongo::OID s_id = mongo::OID::gen();
      create_settings_builder.append("_id", s_id);
      long long t = static_cast<long long>(time(NULL));
      create_settings_builder.append("date", t);
      create_settings_builder.append("initialized", true);
      create_settings_builder.append("version", Version::version());
      mongo::BSONObj s = create_settings_builder.obj();

      c->insert(helpers::collection_name(Collections::SETTINGS()), s);
      if (!c->getLastError().empty()) {
        msg = c->getLastError();
        c.done();
        return false;
      }

      std::string column_id;

      if (!dba::columns::create_column_type_simple("CHROMOSOME", utils::normalize_name("CHROMOSOME"),
          "Chromosome name column. This column should be used to store the Chromosome value in all experiments and annotations",
          utils::normalize_name("Chromosome name column. This column should be used to store the Chromosome value in all experiments and annotations"),
          "string", user_key, column_id, msg)) {
        return false;
      }
      if (!dba::columns::create_column_type_simple("START", utils::normalize_name("START"),
          "Region Start column. This column should be used to store the Region Start position all experiments and annotations",
          utils::normalize_name("Region Start column. This column should be used to store the Region Start position all experiments and annotations"),
          "integer", user_key, column_id, msg)) {
        return false;
      }
      if (!dba::columns::create_column_type_simple("END", utils::normalize_name("END"),
          "Region End column. This column should be used to store the Region End position all experiments and annotations",
          utils::normalize_name("Region End column. This column should be used to store the Region End position all experiments and annotations"),
          "integer", user_key, column_id, msg)) {
        return false;
      }
      if (!dba::columns::create_column_type_simple("VALUE", utils::normalize_name("VALUE"),
          "Region Value. This column should be used to store the Wig Files Region Values",
          utils::normalize_name("Region Value. This column should be used to store the Wig Files Region Values"),
          "double", user_key, column_id, msg)) {
        return false;
      }
      if (!dba::columns::create_column_type_simple("GTF_SCORE", utils::normalize_name("GTF_SCORE"),
          "A floating point score. The type is a string because the value '.' is also acceptable.",
          utils::normalize_name("A floating point score. The type is a string because the value '.' is also acceptable."),
          "string", user_key, column_id, msg)) {
        return false;
      }
      if (!dba::columns::create_column_type_simple("FEATURE", utils::normalize_name("FEATURE"),
          "A floating point score. The type is a string because the value '.' is also acceptable.",
          utils::normalize_name("A floating point score. The type is a string because the value '.' is also acceptable."),
          "string", user_key, column_id, msg)) {
        return false;
      }
      if (!dba::columns::create_column_type_simple("SOURCE", utils::normalize_name("SOURCE"),
          "Name of the program that generated this feature, or the data source (database or project name).",
          utils::normalize_name("Name of the program that generated this feature, or the data source (database or project name)."),
          "string", user_key, column_id, msg)) {
        return false;
      }
      if (!dba::columns::create_column_type_simple("FRAME", utils::normalize_name("FRAME"),
          "One of '0', '1' or '2'. '0' indicates that the first base of the feature is the first base of a codon, '1' that the second base is the first base of a codon, and so on.. The type is a string because the value '.' is also acceptable.",
          utils::normalize_name("One of '0', '1' or '2'. '0' indicates that the first base of the feature is the first base of a codon, '1' that the second base is the first base of a codon, and so on.. The type is a string because the value '.' is also acceptable."),
          "string", user_key, column_id, msg)) {
        return false;
      }
      if (!dba::columns::create_column_type_simple("GTF_ATTRIBUTES", utils::normalize_name("GTF_ATTRIBUTES"),
          "A semicolon-separated list of tag-value pairs, providing additional information about each feature.",
          utils::normalize_name("A semicolon-separated list of tag-value pairs, providing additional information about each feature."),
          "string", user_key, column_id, msg)) {
        return false;
      }

      // Clear caches
      cv::biosources_cache.invalidate();
      query::invalidate_cache();
      users::invalidate_cache();

      c.done();
      return true;
    }

    bool create_indexes(std::string &msg)
    {
      EPIDB_LOG("Creating Indexes");

      Connection c;
      {
        mongo::BSONObjBuilder index_name;
        index_name.append("norm_name", "hashed");
        c->createIndex(helpers::collection_name(Collections::BIOSOURCES()), index_name.obj());
        if (!c->getLastError().empty()) {
          msg = c->getLastError();
          c.done();
          return false;
        }
      }

      {
        mongo::BSONObjBuilder index_name;
        index_name.append("extra_metadata.ontology_id", "hashed");
        c->createIndex(helpers::collection_name(Collections::BIOSOURCES()), index_name.obj());
        if (!c->getLastError().empty()) {
          msg = c->getLastError();
          c.done();
          return false;
        }
      }

      {
        mongo::BSONObjBuilder index_name;
        index_name.append("norm_biosource_name", "hashed");
        c->createIndex(helpers::collection_name(Collections::TEXT_SEARCH()), index_name.obj());
        if (!c->getLastError().empty()) {
          msg = c->getLastError();
          c.done();
          return false;
        }
      }

      {
        mongo::BSONObjBuilder index_name;
        index_name.append("epidb_id", "hashed");
        c->createIndex(helpers::collection_name(Collections::TEXT_SEARCH()), index_name.obj());
        if (!c->getLastError().empty()) {
          msg = c->getLastError();
          c.done();
          return false;
        }
      }

      {
        mongo::BSONObjBuilder index_name;
        index_name.append("sample_info_norm_biosource_name", "hashed");
        c->createIndex(helpers::collection_name(Collections::TEXT_SEARCH()), index_name.obj());
        if (!c->getLastError().empty()) {
          msg = c->getLastError();
          c.done();
          return false;
        }
      }

      {
        mongo::BSONObjBuilder index_syn_names;
        index_syn_names.append("norm_synonym", "hashed");
        c->createIndex(helpers::collection_name(Collections::BIOSOURCE_SYNONYM_NAMES()), index_syn_names.obj());
        if (!c->getLastError().empty()) {
          msg = c->getLastError();
          c.done();
          return false;
        }
      }

      {

        mongo::BSONObjBuilder index_name;
        index_name.append("name", "hashed");
        c->createIndex(helpers::collection_name(Collections::BIOSOURCE_SYNONYMS()), index_name.obj());
        if (!c->getLastError().empty()) {
          msg = c->getLastError();
          c.done();
          return false;
        }
      }

      {
        mongo::BSONObjBuilder index_norm_name;
        index_norm_name.append("norm_name", "hashed");
        c->createIndex(helpers::collection_name(Collections::BIOSOURCE_SYNONYMS()), index_norm_name.obj());
        if (!c->getLastError().empty()) {
          msg = c->getLastError();
          c.done();
          return false;
        }
      }

      {
        mongo::BSONObjBuilder index_syn;
        index_syn.append("synonym", "hashed");
        c->createIndex(helpers::collection_name(Collections::BIOSOURCE_SYNONYMS()), index_syn.obj());
        if (!c->getLastError().empty()) {
          msg = c->getLastError();
          c.done();
          return false;
        }
      }

      {
        mongo::BSONObjBuilder index_name;
        index_name.append("norm_biosource_name", "hashed");
        c->createIndex(helpers::collection_name(Collections::BIOSOURCE_EMBRACING()), index_name.obj());
        if (!c->getLastError().empty()) {
          msg = c->getLastError();
          c.done();
          return false;
        }
      }

      {
        mongo::BSONObjBuilder index_syn;
        index_syn.append("norm_biosource_embracing", "hashed");
        c->createIndex(helpers::collection_name(Collections::BIOSOURCE_EMBRACING()), index_syn.obj());
        if (!c->getLastError().empty()) {
          msg = c->getLastError();
          c.done();
          return false;
        }
      }

      {
        mongo::BSONObjBuilder index_syn;
        index_syn.append("subs", 1);
        c->createIndex(helpers::collection_name(Collections::BIOSOURCE_EMBRACING()), index_syn.obj());
        if (!c->getLastError().empty()) {
          msg = c->getLastError();
          c.done();
          return false;
        }
      }

      {

        mongo::BSONObjBuilder index_name;
        index_name.append("norm_name", "hashed");
        c->createIndex(helpers::collection_name(Collections::TECHNIQUES()), index_name.obj());
        if (!c->getLastError().empty()) {
          msg = c->getLastError();
          c.done();
          return false;
        }
      }

      {
        mongo::BSONObjBuilder index_name;
        index_name.append("name", "hashed");
        c->createIndex(helpers::collection_name(Collections::EXPERIMENTS()), index_name.obj());
        if (!c->getLastError().empty()) {
          msg = c->getLastError();
          c.done();
          return false;
        }
      }

      {
        mongo::BSONObjBuilder index_name;
        index_name.append("norm_name", "hashed");
        c->createIndex(helpers::collection_name(Collections::EXPERIMENTS()), index_name.obj());
        if (!c->getLastError().empty()) {
          msg = c->getLastError();
          c.done();
          return false;
        }
      }

      {
        mongo::BSONObjBuilder index_name;
        index_name.append("norm_genome", "hashed");
        c->createIndex(helpers::collection_name(Collections::EXPERIMENTS()), index_name.obj());
        if (!c->getLastError().empty()) {
          msg = c->getLastError();
          c.done();
          return false;
        }
      }

      {
        mongo::BSONObjBuilder index_name;
        index_name.append("norm_epigenetic_mark", "hashed");
        c->createIndex(helpers::collection_name(Collections::EXPERIMENTS()), index_name.obj());
        if (!c->getLastError().empty()) {
          msg = c->getLastError();
          c.done();
          return false;
        }
      }

      {
        mongo::BSONObjBuilder index_name;
        index_name.append("sample_id", "hashed");
        c->createIndex(helpers::collection_name(Collections::EXPERIMENTS()), index_name.obj());
        if (!c->getLastError().empty()) {
          msg = c->getLastError();
          c.done();
          return false;
        }
      }

      {
        mongo::BSONObjBuilder index_name;
        index_name.append("norm_technique", "hashed");
        c->createIndex(helpers::collection_name(Collections::EXPERIMENTS()), index_name.obj());
        if (!c->getLastError().empty()) {
          msg = c->getLastError();
          c.done();
          return false;
        }
      }

      {
        mongo::BSONObjBuilder index_name;
        index_name.append("norm_project", "hashed");
        c->createIndex(helpers::collection_name(Collections::EXPERIMENTS()), index_name.obj());
        if (!c->getLastError().empty()) {
          msg = c->getLastError();
          c.done();
          return false;
        }
      }

      {
        mongo::BSONObjBuilder index_name;
        index_name.append("key", "hashed");
        c->createIndex(helpers::collection_name(Collections::USERS()), index_name.obj());
        if (!c->getLastError().empty()) {
          msg = c->getLastError();
          c.done();
          return false;
        }
      }

      c.done();

      return true;
    }

    bool create_chromosome_collection(const std::string &genome_norm_name, const std::string &chromosome,
                                      std::string &msg)
    {
      Connection c;
      mongo::BSONObj info;

      std::string collection = helpers::region_collection_name(genome_norm_name, chromosome);
      std::string collection_name = collection.substr(config::DATABASE_NAME().length() + 1);
      EPIDB_LOG("Creating collection: " << collection_name);

      if (!c->createCollection(collection, 0, false, 0, &info)) {
        EPIDB_LOG_ERR("Problem creating collection '" << collection_name << "' error: " << info.toString());
        msg = "Error while creating data collection";
        c.done();
        return false;
      }

      // Unset powerOf2
      mongo::BSONObjBuilder unsetPowerOf2SizesBuilder;
      unsetPowerOf2SizesBuilder.append("collMod", collection_name);
      unsetPowerOf2SizesBuilder.append("usePowerOf2Sizes", false);
      mongo::BSONObj unsetPowerOf2Sizes = unsetPowerOf2SizesBuilder.obj();
      EPIDB_LOG_DBG("Unseting PowerOf2Sizes: " << unsetPowerOf2Sizes.toString());
      if (!c->runCommand(config::DATABASE_NAME(), unsetPowerOf2Sizes, info)) {
        EPIDB_LOG_ERR("Unseting PowerOf2Sizes '" << collection_name << "' error: " << info.toString());
        msg = "Error while setting data collection";
        c.done();
        return false;
      }

      // Set indexes
      EPIDB_LOG_DBG("Creating index for " << collection);
      mongo::BSONObjBuilder dataset_start_end_index;
      dataset_start_end_index.append(KeyMapper::DATASET(), 1);
      dataset_start_end_index.append(KeyMapper::START(), 1);
      dataset_start_end_index.append(KeyMapper::END(), 1);
      c->createIndex(collection, dataset_start_end_index.obj());
      if (!c->getLastError().empty()) {
        msg = c->getLastError();
        EPIDB_LOG_ERR("Indexing on DATASET, START and END at '" << collection << "' error: " << msg);
        c.done();
        return false;
      }

      EPIDB_LOG_DBG("Creating index for " << collection);
      mongo::BSONObjBuilder start_end_index;
      start_end_index.append(KeyMapper::START(), 1);
      start_end_index.append(KeyMapper::END(), 1);
      c->createIndex(collection, start_end_index.obj());
      if (!c->getLastError().empty()) {
        msg = c->getLastError();
        EPIDB_LOG_ERR("Indexing on START and END at '" << collection << "' error: " << msg);
        c.done();
        return false;
      }

      if (config::sharding()) {
        EPIDB_LOG_DBG("Setting sharding for collection " << collection);

        mongo::BSONObjBuilder builder;
        builder.append("shardCollection", collection);
        builder.append("key", BSON(KeyMapper::DATASET() << "hash"));
        mongo::BSONObj objShard = builder.obj();
        mongo::BSONObj info;
        EPIDB_LOG_DBG("Sharding: " << objShard.toString());

        if (!c->runCommand("admin", objShard, info)) {
          EPIDB_LOG_ERR("Sharding on '" << collection << "' error: " << info.toString());
          msg = "Error while sharding data. Check if you are connected to a MongoDB cluster with sharding enabled for the database '" + config::DATABASE_NAME() + "' or disable sharding: --nosharding";
          c.done();
          return false;
        }

        EPIDB_LOG_DBG("Index and Sharding for " << collection << " done");
      }
      c.done();
      return true;
    }


    bool add_genome(const std::string &name, const std::string &norm_name,
                    const std::string &description, const std::string &norm_description,
                    const parser::ChromosomesInfo &genome_info,
                    const std::string &user_key, const std::string &ip,
                    std::string &genome_id, std::string &msg)
    {
      {
        int id;
        if (!helpers::get_increment_counter("genomes", id, msg) ||
            !helpers::notify_change_occurred(Collections::GENOMES(), msg)) {
          return false;
        }
        genome_id = "g" + utils::integer_to_string(id);
      }

      mongo::BSONObjBuilder search_data_builder;
      search_data_builder.append("_id", genome_id);
      search_data_builder.append("name", name);
      search_data_builder.append("norm_name", norm_name);
      search_data_builder.append("description", description);
      search_data_builder.append("norm_description", norm_description);

      mongo::BSONObj search_data = search_data_builder.obj();
      mongo::BSONObjBuilder create_genome_builder;
      create_genome_builder.appendElements(search_data);

      mongo::BSONArrayBuilder ab;
      for (const auto &chr : genome_info) {
        mongo::BSONObjBuilder chromosome_builder;
        chromosome_builder.append("name", chr.first);
        chromosome_builder.append("size", (int) chr.second);
        ab.append(chromosome_builder.obj());

        if (!create_chromosome_collection(name, chr.first, msg)) {
          return false;
        }
      }
      create_genome_builder.append("chromosomes", ab.arr());

      utils::IdName user;
      if (!users::get_user(user_key, user, msg)) {
        return false;
      }
      create_genome_builder.append("user", user.id);
      mongo::BSONObj cem = create_genome_builder.obj();

      Connection c;
      c->insert(helpers::collection_name(Collections::GENOMES()), cem);
      if (!c->getLastError().empty()) {
        msg = c->getLastError();
        c.done();
        return false;
      }
      c.done();

      if (!search::insert_full_text(Collections::GENOMES(), genome_id, search_data, msg)) {
        return false;
      }

      DatasetId id = DATASET_EMPTY_ID;

      ChromosomeRegionsList chromosome_regions_list;
      for (const auto &chr : genome_info) {
        Regions regions = Regions(1);
        regions.emplace_back(build_simple_region(0, chr.second, id));
        ChromosomeRegions chromosome_regions(chr.first, std::move(regions));
        chromosome_regions_list.push_back(std::move(chromosome_regions));
      }

      std::string ann_name = "Chromosomes size for " + name;
      std::string ann_norm_name = utils::normalize_annotation_name(ann_name);
      std::string ann_description = "Chromosomes and sizes of the genome " + name + " (" + description + ")";
      std::string ann_norm_description = utils::normalize_name(ann_description);
      datatypes::Metadata extra_metadata;
      std::string annotation_id;

      if (!insert_annotation(ann_name, ann_norm_name, name, norm_name, ann_description, ann_norm_description, extra_metadata,
                             user_key, ip, chromosome_regions_list, parser::FileFormat::default_format(),
                             annotation_id, msg)) {
        return false;
      }

      return true;
    }

    bool add_epigenetic_mark(const std::string &name, const std::string &norm_name,
                             const std::string &description, const std::string &norm_description,
                             const datatypes::Metadata &extra_metadata,
                             const std::string &user_key,
                             std::string &epigenetic_mark_id, std::string &msg)
    {
      {
        int id;
        if (!helpers::get_increment_counter("epigenetic_marks", id, msg) ||
            !helpers::notify_change_occurred(Collections::EPIGENETIC_MARKS(), msg)) {
          return false;
        }
        epigenetic_mark_id = "em" + utils::integer_to_string(id);
      }

      mongo::BSONObjBuilder search_data_builder;
      search_data_builder.append("_id", epigenetic_mark_id);
      search_data_builder.append("name", name);
      search_data_builder.append("norm_name", norm_name);
      search_data_builder.append("description", description);
      search_data_builder.append("norm_description", norm_description);

      mongo::BSONObjBuilder metadata_builder;
      for (auto cit = extra_metadata.begin(); cit != extra_metadata.end(); ++cit) {
        metadata_builder.append(cit->first, cit->second);
      }
      search_data_builder.append("extra_metadata", metadata_builder.obj());

      mongo::BSONObj search_data = search_data_builder.obj();
      mongo::BSONObjBuilder create_epi_mark_builder;
      create_epi_mark_builder.appendElements(search_data);

      utils::IdName user;
      if (!users::get_user(user_key, user, msg)) {
        return false;
      }
      create_epi_mark_builder.append("user", user.id);
      mongo::BSONObj cem = create_epi_mark_builder.obj();

      Connection c;
      c->insert(helpers::collection_name(Collections::EPIGENETIC_MARKS()), cem);
      if (!c->getLastError().empty()) {
        msg = c->getLastError();
        c.done();
        return false;
      }
      c.done();

      if (!search::insert_full_text(Collections::EPIGENETIC_MARKS(), epigenetic_mark_id, search_data, msg)) {
        return false;
      }

      return true;
    }

    bool add_biosource(const std::string &name, const std::string &norm_name,
                       const std::string &description, const std::string &norm_description,
                       const datatypes::Metadata &extra_metadata,
                       const std::string &user_key,
                       std::string &biosource_id, std::string &msg)
    {
      {
        int id;
        if (!helpers::get_increment_counter("biosources", id, msg) ||
            !helpers::notify_change_occurred(Collections::BIOSOURCES(), msg)) {
          return false;
        }
        biosource_id = "bs" + utils::integer_to_string(id);
      }

      mongo::BSONObjBuilder search_data_builder;
      search_data_builder.append("_id", biosource_id);
      search_data_builder.append("name", name);
      search_data_builder.append("norm_name", norm_name);
      search_data_builder.append("description", description);
      search_data_builder.append("norm_description", norm_description);

      mongo::BSONObjBuilder metadata_builder;
      for (auto cit = extra_metadata.begin(); cit != extra_metadata.end(); ++cit) {
        metadata_builder.append(cit->first, cit->second);
      }
      search_data_builder.append("extra_metadata", metadata_builder.obj());

      mongo::BSONObj search_data = search_data_builder.obj();
      mongo::BSONObjBuilder create_biosource_builder;
      create_biosource_builder.appendElements(search_data);

      utils::IdName user;
      if (!users::get_user(user_key, user, msg)) {
        return false;
      }
      create_biosource_builder.append("user", user.id);
      mongo::BSONObj cem = create_biosource_builder.obj();

      Connection c;
      c->insert(helpers::collection_name(Collections::BIOSOURCES()), cem);
      if (!c->getLastError().empty()) {
        msg = c->getLastError();
        c.done();
        return false;
      }

      if (!search::insert_full_text(Collections::BIOSOURCES(), biosource_id, search_data, msg)) {
        c.done();
        return false;
      }

      c.done();
      return true;
    }

    bool add_technique(const std::string &name, const std::string &norm_name,
                       const std::string &description, const std::string &norm_description,
                       const datatypes::Metadata &extra_metadata,
                       const std::string &user_key,
                       std::string &technique_id, std::string &msg)
    {
      {
        int id;
        if (!helpers::get_increment_counter("techniques", id, msg) ||
            !helpers::notify_change_occurred(Collections::TECHNIQUES(), msg)) {
          return false;
        }
        technique_id = "t" + utils::integer_to_string(id);
      }

      mongo::BSONObjBuilder search_data_builder;
      search_data_builder.append("_id", technique_id);
      search_data_builder.append("name", name);
      search_data_builder.append("norm_name", norm_name);
      search_data_builder.append("description", description);
      search_data_builder.append("norm_description", norm_description);

      mongo::BSONObjBuilder metadata_builder;
      datatypes::Metadata::const_iterator cit;
      for (cit = extra_metadata.begin(); cit != extra_metadata.end(); ++cit) {
        metadata_builder.append(cit->first, cit->second);
      }
      search_data_builder.append("extra_metadata", metadata_builder.obj());


      mongo::BSONObj search_data = search_data_builder.obj();
      mongo::BSONObjBuilder create_technique_builder;
      create_technique_builder.appendElements(search_data);

      utils::IdName user;
      if (!users::get_user(user_key, user, msg)) {
        return false;
      }
      create_technique_builder.append("user", user.id);
      mongo::BSONObj cem = create_technique_builder.obj();

      Connection c;
      c->insert(helpers::collection_name(Collections::TECHNIQUES()), cem);
      if (!c->getLastError().empty()) {
        msg = c->getLastError();
        c.done();
        return false;
      }

      if (!search::insert_full_text(Collections::TECHNIQUES(), technique_id, search_data, msg)) {
        c.done();
        return false;
      }

      c.done();
      return true;
    }

    bool add_sample(const std::string &biosource_name, const std::string &norm_biosource_name,
                    const datatypes::Metadata &metadata,
                    const std::string &user_key,
                    std::string &sample_id, std::string &msg)
    {
      mongo::BSONObjBuilder data_builder;
      data_builder.append("biosource_name", biosource_name);
      data_builder.append("norm_biosource_name", norm_biosource_name);

      std::map<std::string, std::string> names_values;

      std::string err_msg;
      bool err = false;
      for (const datatypes::Metadata::value_type &kv : metadata) {
        if (names_values.find(kv.first) != names_values.end()) {
          msg = "Field " + kv.first + " is duplicated.";
          return false;
        }

        names_values[kv.first] = kv.second;
      }

      if (err) {
        msg = err_msg;
        return false;
      }

      for (const auto &name_value : names_values) {
        std::string norm_title = "norm_" + name_value.first;
        std::string norm_value = utils::normalize_name(name_value.second);
        data_builder.append(name_value.first, name_value.second);
        data_builder.append(norm_title, norm_value);
      }

      mongo::BSONObj data = data_builder.obj();

      Connection c;

      // If we already have a sample with exactly the same information
      auto cursor = c->query(helpers::collection_name(Collections::SAMPLES()), data);
      if (cursor->more()) {
        mongo::BSONObj o = cursor->next();
        sample_id = o["_id"].str();
        c.done();
        return true;
      }

      int id;
      if (!helpers::get_increment_counter("samples", id, msg) ||
          !helpers::notify_change_occurred(Collections::SAMPLES(), msg)) {
        return false;
      }
      sample_id = "s" + utils::integer_to_string(id);

      mongo::BSONObjBuilder create_sample_builder;
      create_sample_builder.append("_id", sample_id);
      create_sample_builder.appendElements(data);

      utils::IdName user;
      if (!users::get_user(user_key, user, msg)) {
        return false;
      }
      create_sample_builder.append("user", user.id);
      mongo::BSONObj cem = create_sample_builder.obj();

      c->insert(helpers::collection_name(Collections::SAMPLES()), cem);
      if (!c->getLastError().empty()) {
        msg = c->getLastError();
        c.done();
        return false;
      }
      c.done();

      if (!search::insert_full_text(Collections::SAMPLES(), sample_id, data, msg)) {
        return false;
      }

      return true;
    }

    bool add_chromosome_sequence(const std::string &genome, const std::string &norm_genome,
                                 const std::string &chromosome,
                                 const std::string &sequence,
                                 const std::string &user_key, std::string &msg)
    {
      std::string filename = norm_genome + "." + chromosome;

      Connection c;

      // check for possible duplicate
      auto data_cursor =
        c->query(helpers::collection_name(Collections::SEQUENCES()) + ".files",
                 BSON("filename" << filename), 1);

      if (data_cursor->more()) {
        c.done();
        msg = "Sequence for chromosome " + chromosome + " of " + genome + " already uploaded.";
        return false;
      }

      // insert sequence
      mongo::GridFS gfs(c.conn(), config::DATABASE_NAME(), Collections::SEQUENCES());

      gfs.setChunkSize(2 << 12); // 8KB

      mongo::BSONObj file = gfs.storeFile(sequence.c_str(), sequence.size(), filename);

      if (!c->getLastError().empty()) {
        msg = c->getLastError();
        c.done();
        return false;
      }

      c->update(helpers::collection_name(Collections::GENOMES()),
                BSON("norm_name" << norm_genome << "chromosomes.name" << chromosome),
                BSON("$set" << BSON("chromosomes.$.sequence_file" << filename)), false, true);

      if (!c->getLastError().empty()) {
        msg = c->getLastError();
        c.done();
        return false;
      }

      c.done();
      return true;
    }

    bool count_experiments(unsigned long long &size, const std::string &user_key, std::string &msg)
    {
      return helpers::collection_size(Collections::EXPERIMENTS(), size, msg);
    }

    bool is_valid_biosource_name(const std::string &name, const std::string &norm_name, std::string &msg)
    {
      if (exists::biosource(norm_name)) {
        std::string e = Error::m(ERR_DUPLICATED_BIOSOURCE_NAME, name);
        EPIDB_LOG_TRACE(e);
        msg = e;
        return false;
      }

      if (exists::biosource_synonym(norm_name)) {
        std::string e = Error::m(ERR_DUPLICATED_BIOSOURCE_NAME, name);
        EPIDB_LOG_TRACE(e);
        msg = e;
        return false;
      }

      return true;
    }

    bool is_valid_technique_name(const std::string &name, const std::string &norm_name, std::string &msg)
    {
      if (exists::technique(norm_name)) {
        std::string s = Error::m(ERR_DUPLICATED_TECHNIQUE_NAME, name);
        EPIDB_LOG_TRACE(s);
        msg = s;
        return false;
      }
      return true;
    }

    bool is_valid_epigenetic_mark(const std::string &name, const std::string &norm_name, std::string &msg)
    {
      if (exists::epigenetic_mark(norm_name)) {
        std::string s = Error::m(ERR_DUPLICATED_EPIGENETIC_MARK_NAME, name);
        EPIDB_LOG_TRACE(s);
        msg = s;
        return false;
      }
      return true;
    }

    bool is_project_valid(const std::string &name, const std::string &norm_name, std::string &msg)
    {
      if (exists::project(norm_name)) {
        std::string s = Error::m(ERR_DUPLICATED_PROJECT_NAME, name);
        EPIDB_LOG_TRACE(s);
        msg = s;
        return false;
      }
      return true;
    }

    bool is_valid_genome(const std::string &genome, const std::string &norm_genome, std::string &msg)
    {
      if (exists::genome(norm_genome)) {
        std::string s = Error::m(ERR_DUPLICATED_GENOME_NAME, genome);
        EPIDB_LOG_TRACE(s);
        msg = s;
        return false;
      }
      return true;
    }

    bool set_biosource_parent(const std::string &biosource_more_embracing, const std::string &norm_biosource_more_embracing,
                              const std::string &biosource_less_embracing, const std::string &norm_biosource_less_embracing,
                              bool more_embracing_is_syn, const bool less_embracing_is_syn,
                              const std::string &user_key, std::string &msg)
    {
      return cv::set_biosource_parent(biosource_more_embracing, norm_biosource_more_embracing,
                                      biosource_less_embracing, norm_biosource_less_embracing,
                                      more_embracing_is_syn, less_embracing_is_syn, user_key, msg);
    }

    bool get_biosource_children(const std::string &biosource_name, const std::string &norm_biosource_name,
                                bool is_biosource, const std::string &user_key,
                                std::vector<utils::IdName> &related_biosources, std::string &msg)
    {
      std::vector<std::string> norm_subs;

      if (!cv::get_biosource_children(biosource_name, norm_biosource_name, is_biosource, user_key, norm_subs, msg)) {
        return false;
      }

      for (const std::string &norm_sub : norm_subs) {
        utils::IdName sub_biosource_name;
        if (!helpers::get_name(Collections::BIOSOURCES(), norm_sub, sub_biosource_name, msg)) {
          return false;
        }
        related_biosources.push_back(sub_biosource_name);
      }
      return true;
    }

    bool get_biosource_parents(const std::string &biosource_name, const std::string &norm_biosource_name,
                               bool is_biosource, const std::string &user_key,
                               std::vector<utils::IdName> &related_biosources, std::string &msg)
    {
      std::vector<std::string> norm_subs;

      if (!cv::get_biosource_parents(biosource_name, norm_biosource_name, is_biosource, user_key, norm_subs, msg)) {
        return false;
      }

      for (const std::string &norm_sub : norm_subs) {
        utils::IdName sub_biosource_name;
        if (!helpers::get_name(Collections::BIOSOURCES(), norm_sub, sub_biosource_name, msg)) {
          return false;
        }
        related_biosources.push_back(sub_biosource_name);
      }
      return true;
    }

    const std::string build_pattern_annotation_name(const std::string &pattern, const std::string &genome, const bool overlap)
    {
      std::string name = "Pattern " + pattern;
      if (overlap) {
        name = name + " (overlap)";
      } else {
        name = name + " (non-overlap)";
      }
      return name + " in the genome " + genome;
    }

    bool process_pattern(const std::string &genome, const std::string &pattern, const bool overlap,
                         const std::string &user_key, const std::string &ip,
                         utils::IdName &annotation_id_name, std::string &msg)
    {
      std::string norm_genome = utils::normalize_name(genome);
      std::string name = build_pattern_annotation_name(pattern, genome, overlap);
      std::string norm_name = utils::normalize_annotation_name(name);

      mongo::BSONObjBuilder builder;
      builder.append("norm_name", norm_name);
      builder.append("norm_genome", norm_genome);
      mongo::BSONObj o = builder.obj();

      std::vector<mongo::BSONObj> results;
      if (!helpers::get(Collections::ANNOTATIONS(), o, results, msg)) {
        return false;
      }

      if (!results.empty()) {
        annotation_id_name.id = results[0]["_id"].str();
        annotation_id_name.name = results[0]["name"].str();
        return true;
      }

      std::vector<genomes::ChromosomeInfo> chromosomes;
      if (!get_chromosomes(norm_genome, chromosomes, msg)) {
        return false;
      }

      retrieve::SequenceRetriever retriever;
      std::vector<std::string> missing;
      for (const genomes::ChromosomeInfo &chromosome_info : chromosomes) {
        if (!retriever.exists(genome, chromosome_info.name)) {
          missing.push_back(chromosome_info.name);
        }
      }
      if (!missing.empty()) {
        msg = "There is not sequence for the chromosomes '" + utils::vector_to_string(missing) + "'' of the genome " + genome + ". Please upload using 'upload_chromosome' command.";
      }

      ChromosomeRegionsList pattern_regions;
      for (const genomes::ChromosomeInfo &chromosome_info : chromosomes) {
        std::string sequence;
        if (!retriever.retrieve(norm_genome, chromosome_info.name, 0, chromosome_info.size, sequence, msg)) {
          return false;
        }

        algorithms::PatternFinder pf(sequence, pattern);
        Regions regions;
        if (overlap) {
          regions = pf.overlap_regions();
        } else {
          regions = pf.non_overlap_regions();
        }
        ChromosomeRegions chromosome_regions(chromosome_info.name, std::move(regions));
        pattern_regions.push_back(std::move(chromosome_regions));
      }

      std::string description = "All localization of the pattern '" + pattern + "' in the genome " + genome;
      std::string norm_description = utils::normalize_name(description);

      datatypes::Metadata extra_metadata;
      extra_metadata["pattern"] = pattern;
      if (overlap) {
        extra_metadata["overlap-style"] = "overlap";
      } else {
        extra_metadata["overlap-style"] = "non-overlap";
      }

      std::string annotation_id;
      if (!insert_annotation(name, norm_name, genome, norm_genome, description, norm_description, extra_metadata,
                             user_key, ip, pattern_regions, parser::FileFormat::default_format(),
                             annotation_id, msg)) {
        return false;
      }

      annotation_id_name.name = name;
      annotation_id_name.id = annotation_id;

      return true;
    }

    bool find_annotation_pattern(const std::string &genome, const std::string &pattern, const bool overlap,
                                 DatasetId &dataset_id, std::string &msg)
    {

      mongo::BSONObjBuilder annotations_query_builder;

      std::string name = build_pattern_annotation_name(pattern, genome, overlap);
      std::string norm_name = utils::normalize_annotation_name(name);
      std::string norm_genome = utils::normalize_name(genome);


      annotations_query_builder.append("norm_name", norm_name);
      annotations_query_builder.append("norm_genome", norm_genome);
      mongo::BSONObjBuilder metadata_builder;
      if (overlap) {
        metadata_builder.append("overlap-style", "overlap");
      } else {
        metadata_builder.append("overlap-style", "non-overlap");
      }
      metadata_builder.append("pattern", pattern);
      annotations_query_builder.append("extra_metadata", metadata_builder.obj());
      annotations_query_builder.append("upload_info.done", true);

      Connection c;

      mongo::BSONObj annotation_query = annotations_query_builder.obj();
      auto cursor = c->query(helpers::collection_name(Collections::ANNOTATIONS()), annotation_query);

      if (cursor->more()) {
        mongo::BSONObj p = cursor->next();
        dataset_id = p.getField(KeyMapper::DATASET()).Int();
        c.done();
        return true;
      } else {
        if (overlap) {
          msg = Error::m(ERR_INVALID_PRA_PROCESSED_ANNOTATION_NAME, "overlapped pattern", pattern, genome);
        } else {
          msg = Error::m(ERR_INVALID_PRA_PROCESSED_ANNOTATION_NAME, "non-overlapped pattern", pattern, genome);
        }
        c.done();
        return false;
      }
    }

    bool datatable(const std::string collection,
                   const long long start, const long long length,
                   const std::string global_search, const std::string sort_column, const std::string sort_direction,
                   const std::string has_filter, const std::string user_key)
    {

      Connection c;

      mongo::BSONObjBuilder query_builder;
      mongo::Query query(query_builder.obj());
      auto cursor = c->query(helpers::collection_name(collection), query, length, start);

      return true;
    }

  }
}
