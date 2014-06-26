//
//  dba.cpp
//  epidb
//
//  Created by Felipe Albrecht on 01.06.13.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include <map>
#include <string>
#include <sstream>
#include <vector>

#include <time.h>
#include <math.h>

#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>

#include <mongo/bson/bson.h>
#include <mongo/client/dbclient.h>

#include "../algorithms/patterns.hpp"
#include "../extras/utils.hpp"
#include "../parser/field_type.hpp"
#include "../parser/genome_data.hpp"
#include "../parser/parser_factory.hpp"
#include "../parser/wig_parser.hpp"

#include "dba.hpp"
#include "config.hpp"
#include "collections.hpp"
#include "controlled_vocabulary.hpp"
#include "full_text.hpp"
#include "genomes.hpp"
#include "helpers.hpp"
#include "insert.hpp"
#include "key_mapper.hpp"
#include "queries.hpp"
#include "retrieve.hpp"
#include "info.hpp"

#include "../errors.hpp"
#include "../regions.hpp"
#include "../log.hpp"

namespace epidb {
  namespace dba {

    bool is_initialized(bool &ret, std::string &msg)
    {
      return helpers::check_exist(Collections::SETTINGS(), "initialized", true, ret, msg);
    }

    bool init_system(const std::string &name, const std::string &email, const std::string &institution,
                     const std::string &key, std::string &msg)
    {
      mongo::ScopedDbConnection c(config::get_mongodb_server());

      if (config::sharding()) {
        mongo::BSONObjBuilder builder;
        builder.append("enableSharding", dba::DATABASE_NAME);
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
          EPIDB_LOG("Shard enabled for: " << dba::DATABASE_NAME << " info: " << info.toString());
        }

        mongo::BSONObjBuilder movePrimaryBuilder;
        movePrimaryBuilder.append("movePrimary", dba::DATABASE_NAME);
        movePrimaryBuilder.append("to", "shard0000");
        if (!c->runCommand("admin", movePrimaryBuilder.obj(), info)) {
          EPIDB_LOG_WARN("Error movePrimary: " << info.toString());
        } else {
          EPIDB_LOG("Primary moved: " << info.toString());
        }

        // db.settings.save( { _id:"chunksize", value: <sizeInMB> } )
        c->update("config.settings", QUERY("_id" << "chunksize"), BSON("$set" << BSON("value" << (unsigned) config::chunk_size())));
        if (!c->getLastError().empty()) {
          msg = "Error Setting ChuckSize: " + c->getLastError();
          EPIDB_LOG_ERR(msg);
          c.done();
          return false;
        }
      }

      std::string user_id;
      if (!add_user(name, email, institution, key, user_id, msg)) {
        return false;
      }
      if (!set_user_admin(user_id, true, msg)) {
        return false;
      }

      mongo::BSONObjBuilder index_name;
      index_name.append("key", 1);
      c->ensureIndex(helpers::collection_name(Collections::USERS()), index_name.obj());
      if (!c->getLastError().empty()) {
        msg = c->getLastError();
        c.done();
        return false;
      }

      mongo::BSONObjBuilder create_settings_builder;
      mongo::OID s_id = mongo::OID::gen();
      create_settings_builder.append("_id", s_id);
      long long t = static_cast<long long>(time(NULL));
      create_settings_builder.append("date", t);
      create_settings_builder.append("initialized", true);
      mongo::BSONObj s = create_settings_builder.obj();

      c->insert(helpers::collection_name(Collections::SETTINGS()), s);
      if (!c->getLastError().empty()) {
        msg = c->getLastError();
        c.done();
        return false;
      }

      std::string column_id;

      if (!dba::columns::create_column_type_simple("CHROMOSOME", utils::normalize_name("CHROMOSOME"),
          /* description */ "" , /* norm_description */ "", /* ignore_if */ "",
          "string",  key, column_id, msg)) {
        return false;
      }
      if (!dba::columns::create_column_type_simple("START", utils::normalize_name("START"),
          /* description */ "" , /* norm_description */ "", /* ignore_if */ "",
          "integer",  key, column_id, msg)) {
        return false;
      }
      if (!dba::columns::create_column_type_simple("END", utils::normalize_name("END"),
          /* description */ "" , /* norm_description */ "", /* ignore_if */ "",
          "integer",  key, column_id, msg)) {
        return false;
      }

      c.done();
      return true;
    }

    bool set_user_admin(const std::string &user_id, const bool value, std::string &msg)
    {
      mongo::BSONObj o = BSON("findandmodify" << Collections::USERS() <<
                              "query" << BSON("_id" << user_id) <<
                              "update" << BSON("$set" << BSON("admin" << value)));

      mongo::ScopedDbConnection c(config::get_mongodb_server());
      mongo::BSONObj info;
      bool result = c->runCommand(dba::DATABASE_NAME, o, info);
      if (!result) {
        // TODO: get info error
        msg = "error setting admin in user '" + user_id + "'.";
        c.done();
        return  false;
      }
      c.done();
      return true;
    }

    bool is_admin_key(const std::string &admin_key, bool &ret, std::string &msg)
    {
      mongo::ScopedDbConnection c(config::get_mongodb_server());

      mongo::BSONObjBuilder query_builder;

      query_builder.append("admin", true);
      query_builder.append("key", admin_key);

      mongo::BSONObj query = query_builder.obj();
      long long count = c->count(helpers::collection_name(Collections::USERS()), query);
      if (!c->getLastError().empty()) {
        msg = c->getLastError();
        c.done();
        return false;
      }

      ret = count > 0;
      c.done();
      return true;
    }

    bool add_user(const std::string &name, const std::string &email, const std::string &institution,
                  const std::string &key, std::string &user_id, std::string &msg)
    {
      {
        int id;
        if (!helpers::get_counter("users", id, msg))  {
          return false;
        }
        user_id = "u" + boost::lexical_cast<std::string>(id);
      }

      mongo::BSONObjBuilder create_user_builder;
      create_user_builder.append("_id", user_id);
      create_user_builder.append("name", name);
      create_user_builder.append("email", email);
      create_user_builder.append("institution", institution);
      create_user_builder.append("key", key);
      mongo::BSONObj cu = create_user_builder.obj();

      mongo::ScopedDbConnection c(config::get_mongodb_server());
      c->insert(helpers::collection_name(Collections::USERS()), cu);
      if (!c->getLastError().empty()) {
        msg = c->getLastError();
        c.done();
        return false;
      }

      mongo::BSONObjBuilder index_name;
      index_name.append("key", 1);
      c->ensureIndex(helpers::collection_name(Collections::USERS()), index_name.obj());
      if (!c->getLastError().empty()) {
        msg = c->getLastError();
        c.done();
        return false;
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
        if (!helpers::get_counter("genomes", id, msg))  {
          return false;
        }
        genome_id = "g" + boost::lexical_cast<std::string>(id);
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
      for (parser::ChromosomesInfo::const_iterator it = genome_info.begin(); it != genome_info.end(); it++) {
        mongo::BSONObjBuilder chromosome_builder;
        chromosome_builder.append("name", it->first);
        chromosome_builder.append("size", it->second);
        ab.append(chromosome_builder.obj());
      }

      create_genome_builder.append("chromosomes", ab.arr());

      utils::IdName id_user_name;
      if (!get_user_name(user_key, id_user_name, msg)) {
        return false;
      }
      create_genome_builder.append("user", id_user_name.name);
      mongo::BSONObj cem = create_genome_builder.obj();

      mongo::ScopedDbConnection c(config::get_mongodb_server());
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

      CollectionId id = build_collection_id("Add Genome Regions");

      ChromosomeRegionsList chromosome_regions_list;
      for (parser::ChromosomesInfo::const_iterator it = genome_info.begin(); it != genome_info.end(); it++) {
        Regions regions = build_regions();
        Region region(0, it->second, id);
        regions->push_back(region);
        ChromosomeRegions chromosome_regions(it->first, regions);
        chromosome_regions_list.push_back(chromosome_regions);
      }

      std::string ann_name = name;
      std::string ann_norm_name = utils::normalize_annotation_name(ann_name);
      std::string ann_description = "Chromosomes and sizes of the genome " + name;
      std::string ann_norm_description = utils::normalize_name(description);
      Metadata extra_metadata;
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
                             const std::string &user_key,
                             std::string &epigenetic_mark_id, std::string &msg)
    {
      {
        int id;
        if (!helpers::get_counter("epigenetic_marks", id, msg))  {
          return false;
        }
        epigenetic_mark_id = "em" + boost::lexical_cast<std::string>(id);
      }

      mongo::BSONObjBuilder search_data_builder;
      search_data_builder.append("_id", epigenetic_mark_id);
      search_data_builder.append("name", name);
      search_data_builder.append("norm_name", norm_name);
      search_data_builder.append("description", description);
      search_data_builder.append("norm_description", norm_description);

      mongo::BSONObj search_data = search_data_builder.obj();
      mongo::BSONObjBuilder create_epi_mark_builder;
      create_epi_mark_builder.appendElements(search_data);

      utils::IdName id_user_name;
      if (!get_user_name(user_key, id_user_name, msg)) {
        return false;
      }
      create_epi_mark_builder.append("user", id_user_name.name);
      mongo::BSONObj cem = create_epi_mark_builder.obj();

      mongo::ScopedDbConnection c(config::get_mongodb_server());
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

    bool add_bio_source(const std::string &name, const std::string &norm_name,
                        const std::string &description, const std::string &norm_description,
                        const Metadata &extra_metadata,
                        const std::string &user_key,
                        std::string &bio_source_id, std::string &msg)
    {
      {
        int id;
        if (!helpers::get_counter("bio_sources", id, msg))  {
          return false;
        }
        bio_source_id = "bs" + boost::lexical_cast<std::string>(id);
      }

      mongo::BSONObjBuilder search_data_builder;
      search_data_builder.append("_id", bio_source_id);
      search_data_builder.append("name", name);
      search_data_builder.append("norm_name", norm_name);
      search_data_builder.append("description", description);
      search_data_builder.append("norm_description", norm_description);

      mongo::BSONObjBuilder metadata_builder;
      Metadata::const_iterator cit;
      for (cit = extra_metadata.begin(); cit != extra_metadata.end(); ++cit) {
        metadata_builder.append(cit->first, cit->second);
      }
      search_data_builder.append("extra_metadata", metadata_builder.obj());


      mongo::BSONObj search_data = search_data_builder.obj();
      mongo::BSONObjBuilder create_bio_source_builder;
      create_bio_source_builder.appendElements(search_data);

      utils::IdName id_user_name;
      if (!get_user_name(user_key, id_user_name, msg)) {
        return false;
      }
      create_bio_source_builder.append("user", id_user_name.name);
      mongo::BSONObj cem = create_bio_source_builder.obj();

      mongo::ScopedDbConnection c(config::get_mongodb_server());
      c->insert(helpers::collection_name(Collections::BIO_SOURCES()), cem);
      if (!c->getLastError().empty()) {
        msg = c->getLastError();
        c.done();
        return false;
      }

      mongo::BSONObjBuilder index_name;
      index_name.append("norm_name", 1);
      c->ensureIndex(helpers::collection_name(Collections::BIO_SOURCES()), index_name.obj());
      if (!c->getLastError().empty()) {
        msg = c->getLastError();
        c.done();
        return false;
      }

      if (!search::insert_full_text(Collections::BIO_SOURCES(), bio_source_id, search_data, msg)) {
        c.done();
        return false;
      }

      c.done();
      return true;
    }

    bool add_technique(const std::string &name, const std::string &norm_name,
                       const std::string &description, const std::string &norm_description,
                       const Metadata &extra_metadata,
                       const std::string &user_key,
                       std::string &technique_id, std::string &msg)
    {
      {
        int id;
        if (!helpers::get_counter("techniques", id, msg))  {
          return false;
        }
        technique_id = "t" + boost::lexical_cast<std::string>(id);
      }

      mongo::BSONObjBuilder search_data_builder;
      search_data_builder.append("_id", technique_id);
      search_data_builder.append("name", name);
      search_data_builder.append("norm_name", norm_name);
      search_data_builder.append("description", description);
      search_data_builder.append("norm_description", norm_description);

      mongo::BSONObjBuilder metadata_builder;
      Metadata::const_iterator cit;
      for (cit = extra_metadata.begin(); cit != extra_metadata.end(); ++cit) {
        metadata_builder.append(cit->first, cit->second);
      }
      search_data_builder.append("extra_metadata", metadata_builder.obj());


      mongo::BSONObj search_data = search_data_builder.obj();
      mongo::BSONObjBuilder create_technique_builder;
      create_technique_builder.appendElements(search_data);

      utils::IdName id_user_name;
      if (!get_user_name(user_key, id_user_name, msg)) {
        return false;
      }
      create_technique_builder.append("user", id_user_name.name);
      mongo::BSONObj cem = create_technique_builder.obj();

      mongo::ScopedDbConnection c(config::get_mongodb_server());
      c->insert(helpers::collection_name(Collections::TECHNIQUES()), cem);
      if (!c->getLastError().empty()) {
        msg = c->getLastError();
        c.done();
        return false;
      }

      mongo::BSONObjBuilder index_name;
      index_name.append("norm_name", 1);
      c->ensureIndex(helpers::collection_name(Collections::TECHNIQUES()), index_name.obj());
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

    bool add_sample(const std::string &bio_source_name, const std::string &norm_bio_source_name,
                    const Metadata &metadata,
                    const std::string &user_key,
                    std::string &sample_id, std::string &msg)
    {
      {
        int id;
        if (!helpers::get_counter("samples", id, msg))  {
          return false;
        }
        sample_id = "s" + boost::lexical_cast<std::string>(id);
      }
      mongo::BSONObjBuilder search_data_builder;
      search_data_builder.append("_id", sample_id);
      search_data_builder.append("bio_source_name", bio_source_name);
      search_data_builder.append("norm_bio_source_name", norm_bio_source_name);

      std::map<std::string, std::string> names_values;
      std::map<std::string, std::string>::iterator it;

      std::string err_msg;
      bool err = false;
      BOOST_FOREACH(const Metadata::value_type & kv, metadata) {
        if (kv.second.empty()) {
          msg = "The field " + kv.first + " does not have the right value. (key:value).";
          return false;
        }

        if (names_values.find(kv.first) != names_values.end()) {
          msg = "Field " + kv.first + " is duplicated.";
          return false;
        }

        bool r = false;
        if (!check_sample_field(utils::normalize_name(kv.first), r, msg)) {
          return false;
        } else if (!r) {
          // TODO: return the similar sample_field names
          msg = "Field " + kv.first + " is not registered. Please use add_sample_field command or check the field name.";
          if (err_msg.empty()) {
            err_msg = msg;
          } else {
            err_msg = err_msg + " " + msg;
          }
          err = true;
        } else {
          names_values[kv.first] = kv.second;
        }
      }

      if (err) {
        msg = err_msg;
        return false;
      }

      for (it = names_values.begin(); it != names_values.end(); it++) {
        std::string norm_title = "norm_" + it->first;
        std::string norm_value = utils::normalize_name(it->second);
        search_data_builder.append(it->first, it->second);
        search_data_builder.append(norm_title, norm_value);
      }

      mongo::BSONObj search_data = search_data_builder.obj();
      mongo::BSONObjBuilder create_sample_builder;
      create_sample_builder.appendElements(search_data);

      utils::IdName id_user_name;
      if (!get_user_name(user_key, id_user_name, msg)) {
        return false;
      }
      create_sample_builder.append("user", id_user_name.name);
      mongo::BSONObj cem = create_sample_builder.obj();

      mongo::ScopedDbConnection c(config::get_mongodb_server());
      c->insert(helpers::collection_name(Collections::SAMPLES()), cem);
      if (!c->getLastError().empty()) {
        msg = c->getLastError();
        c.done();
        return false;
      }
      c.done();

      if (!search::insert_full_text(Collections::SAMPLES(), sample_id, search_data, msg)) {
        return false;
      }

      return true;
    }

    bool add_sample_field(const std::string &name, const std::string &norm_name,
                          const std::string &type,
                          const std::string &description, const std::string &norm_description,
                          const std::string &user_key,
                          std::string &sample_field_id, std::string &msg)
    {
      {
        int id;
        if (!helpers::get_counter("sample_fields", id, msg))  {
          return false;
        }
        sample_field_id = "f" + boost::lexical_cast<std::string>(id);
      }
      mongo::BSONObjBuilder create_sample_fields_builder;
      create_sample_fields_builder.append("_id", sample_field_id);
      create_sample_fields_builder.append("name", name);
      create_sample_fields_builder.append("norm_name", norm_name);
      create_sample_fields_builder.append("type", type);
      create_sample_fields_builder.append("norm_name", norm_name);
      create_sample_fields_builder.append("description", description);
      create_sample_fields_builder.append("norm_description", norm_description);

      utils::IdName id_user_name;
      if (!get_user_name(user_key, id_user_name, msg)) {
        return false;
      }
      create_sample_fields_builder.append("user", id_user_name.name);
      mongo::BSONObj cem = create_sample_fields_builder.obj();

      mongo::ScopedDbConnection c(config::get_mongodb_server());
      c->insert(helpers::collection_name(Collections::SAMPLE_FIELDS()), cem);
      if (!c->getLastError().empty()) {
        msg = c->getLastError();
        c.done();
        return false;
      }

      c.done();
      return true;

    }

    bool add_project(const std::string &name, const std::string &norm_name,
                     const std::string &description, const std::string &norm_description,
                     const std::string &user_key,
                     std::string &project_id, std::string &msg)
    {
      {
        int id;
        if (!helpers::get_counter("projects", id, msg))  {
          return false;
        }
        project_id = "p" + boost::lexical_cast<std::string>(id);
      }
      mongo::BSONObjBuilder search_data_builder;
      search_data_builder.append("_id", project_id);
      search_data_builder.append("name", name);
      search_data_builder.append("norm_name", norm_name);
      search_data_builder.append("description", description);
      search_data_builder.append("norm_description", norm_description);

      mongo::BSONObj search_data = search_data_builder.obj();
      mongo::BSONObjBuilder create_project_builder;
      create_project_builder.appendElements(search_data);

      utils::IdName id_user_name;
      if (!get_user_name(user_key, id_user_name, msg)) {
        return false;
      }
      create_project_builder.append("user", id_user_name.name);
      mongo::BSONObj cem = create_project_builder.obj();

      mongo::ScopedDbConnection c(config::get_mongodb_server());
      c->insert(helpers::collection_name(Collections::PROJECTS()), cem);
      if (!c->getLastError().empty()) {
        msg = c->getLastError();
        c.done();
        return false;
      }

      if (!search::insert_full_text(Collections::PROJECTS(), project_id, search_data, msg)) {
        c.done();
        return false;
      }

      c.done();
      return true;
    }

    bool add_chromosome_sequence(const std::string &genome, const std::string &norm_genome,
                                 const std::string &chromosome,
                                 const std::string &sequence,
                                 const std::string &user_key, std::string &msg)
    {
      std::string filename = norm_genome + "." + chromosome;

      mongo::ScopedDbConnection c(config::get_mongodb_server());

      // check for possible duplicate
      std::auto_ptr<mongo::DBClientCursor> data_cursor =
        c->query(helpers::collection_name(Collections::SEQUENCES()) + ".files",
                 QUERY("filename" << filename), 1);

      if (data_cursor->more()) {
        c.done();
        msg = "Sequence for chromosome " + chromosome + " of " + genome + " already uploaded.";
        return false;
      }

      // insert sequence
      mongo::GridFS gfs(c.conn(), DATABASE_NAME, Collections::SEQUENCES());

      gfs.setChunkSize(2 << 12); // 8KB

      mongo::BSONObj file = gfs.storeFile(utils::upper(sequence).c_str(), sequence.size(), filename);

      if (!c->getLastError().empty()) {
        msg = c->getLastError();
        c.done();
        return false;
      }

      c->update(helpers::collection_name(Collections::GENOMES()),
                QUERY("norm_name" << norm_genome << "chromosomes.name" << chromosome),
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
      return helpers::collection_size("experiments", size, msg);
    }

    bool is_valid_bio_source_name(const std::string &name, const std::string &norm_name, std::string &msg)
    {
      bool exists;
      if (!helpers::check_exist(Collections::BIO_SOURCES(), "norm_name", norm_name, exists, msg)) {
        return false;
      }
      if (exists) {
        std::string e = Error::m(ERR_DUPLICATED_BIO_SOURCE_NAME, name.c_str());
        EPIDB_LOG_TRACE(e);
        msg = e;
        return false;
      }
      return true;
    }


    bool is_valid_sample_name(const std::string &name, const std::string &norm_name, std::string &msg)
    {
      bool exists;
      if (!helpers::check_exist(Collections::SAMPLES(), "norm_name", norm_name, exists, msg)) {
        return false;
      }
      if (exists) {
        std::stringstream ss;
        ss << "Sample name '" << name << "' already exists.";
        msg = ss.str();
        return false;
      }
      return true;
    }

    bool is_valid_sample_field_name(const std::string &name, const std::string &norm_name, std::string &msg)
    {
      bool exists;
      if (!helpers::check_exist(Collections::SAMPLE_FIELDS(), "norm_name", norm_name, exists, msg)) {
        return false;
      }
      if (exists) {
        std::stringstream ss;
        ss << "Sample field name '" << name << "' already exists.";
        msg = ss.str();
        return false;
      }
      return true;
    }

    bool is_valid_epigenetic_mark(const std::string &name, const std::string &norm_name, std::string &msg)
    {
      bool exists;
      if (!helpers::check_exist(Collections::EPIGENETIC_MARKS(), "norm_name", norm_name, exists, msg)) {
        return false;
      }
      if (exists) {
        std::string s = Error::m(ERR_DUPLICATED_EPIGENETIC_MARK_NAME, name.c_str());
        EPIDB_LOG_TRACE(s);
        msg = s;
        return false;
      }
      return true;
    }

    bool is_project_valid(const std::string &name, const std::string &norm_name, std::string &msg)
    {
      bool exists;
      if (!helpers::check_exist(Collections::PROJECTS(), "norm_name", norm_name, exists, msg)) {
        return false;
      }
      if (exists) {
        std::stringstream ss;
        ss << "Project '" << name << "' already exists.";
        msg = ss.str();
        return false;
      }
      return true;
    }

    bool is_valid_genome(const std::string &genome, const std::string &norm_genome, std::string &msg)
    {
      bool exists;
      if (!helpers::check_exist(Collections::GENOMES(), "norm_name", norm_genome, exists, msg)) {
        return false;
      }
      if (exists) {
        std::stringstream ss;
        ss << "Genome '" << genome << "' already exists.";
        msg = ss.str();
        return false;
      }
      return true;
    }

    bool is_valid_email(const std::string &email, std::string &msg)
    {
      bool exists;
      if (!helpers::check_exist(Collections::USERS(), "email", email, exists, msg)) {
        return false;
      }
      if (exists) {
        std::stringstream ss;
        ss << "Email '" << email << "' is already being used.";
        msg = ss.str();
        return false;
      }
      return true;
    }

    bool check_user(const std::string &user_key, bool &r, std::string &msg)
    {
      return helpers::check_exist(Collections::USERS(), "key", user_key, r, msg);
    }

    bool check_genome(const std::string &genome, bool &r, std::string &msg)
    {
      std::string norm_genome = utils::normalize_name(genome);
      return helpers::check_exist(Collections::GENOMES(), "norm_name", norm_genome, r, msg);
    }

    bool check_epigenetic_mark(const std::string &epigenetic_mark, bool &r, std::string &msg)
    {
      std::string norm_epigenetic_mark = utils::normalize_name(epigenetic_mark);
      return helpers::check_exist(Collections::EPIGENETIC_MARKS(), "norm_name", norm_epigenetic_mark, r, msg);
    }

    bool sample(const std::string &bio_source_name, bool &r, std::string &msg)
    {
      std::string norm_bio_source_name = utils::normalize_name(bio_source_name);
      return helpers::check_exist(Collections::SAMPLES(), "norm_name", norm_bio_source_name, r, msg);
    }

    bool check_bio_source(const std::string &bio_source_name, bool &r, std::string &msg)
    {
      std::string norm_bio_source_name = utils::normalize_name(bio_source_name);
      return helpers::check_exist(Collections::BIO_SOURCES(), "norm_name", norm_bio_source_name, r, msg);
    }

    bool check_sample_field(const std::string &bio_source_name, bool &r, std::string &msg)
    {
      std::string norm_bio_source_name = utils::normalize_name(bio_source_name);
      return helpers::check_exist(Collections::SAMPLE_FIELDS(), "norm_name", norm_bio_source_name, r, msg);
    }

    bool check_technique(const std::string &technique_name, bool &r, std::string &msg)
    {
      std::string norm_technique_name = utils::normalize_name(technique_name);
      return helpers::check_exist(Collections::TECHNIQUES(), "norm_name", norm_technique_name, r, msg);
    }

    bool check_bio_source_synonym(const std::string &bio_source_synonym, bool &r, std::string &msg)
    {
      std::string norm_bio_source_synonym = utils::normalize_name(bio_source_synonym);
      return helpers::check_exist(Collections::BIO_SOURCE_SYNONYM_NAMES(), "norm_synonym", norm_bio_source_synonym, r, msg);
    }

    bool check_project(const std::string &project, bool &r, std::string &msg)
    {
      std::string norm_project = utils::normalize_name(project);
      return helpers::check_exist(Collections::PROJECTS(), "norm_name", norm_project, r, msg);
    }

    bool check_annotation(const std::string &annotation, const std::string &genome, bool &ok, std::string &msg)
    {
      std::string norm_annotation = utils::normalize_name(annotation);
      std::vector<helpers::QueryPair> query;
      query.push_back(helpers::QueryPair("norm_name", norm_annotation));
      query.push_back(helpers::QueryPair("genome", genome));
      std::vector<mongo::BSONObj> results;

      if (!helpers::get("annotations", query, results, msg)) {
        return false;
      }

      ok = results.size() == 0;

      return true;
    }

    bool check_experiment_name(const std::string &name, const std::string &norm_name, const std::string &user_key,
                               bool &ok, std::string &msg)
    {
      std::vector<helpers::QueryPair> query;
      query.push_back(helpers::QueryPair("norm_name", norm_name));

      utils::IdName id_user_name;
      if (!get_user_name(user_key, id_user_name, msg)) {
        return false;
      }
      query.push_back(helpers::QueryPair("user", id_user_name.name));
      std::vector<mongo::BSONObj> results;

      if (!helpers::get("experiments", query, results, msg)) {
        return false;
      }

      ok = results.size() == 0;

      return true;
    }

    bool check_query(const std::string &user_key, const std::string &query_id, bool &r, std::string &msg)
    {
      utils::IdName id_user_name;
      if (!get_user_name(user_key, id_user_name, msg)) {
        return false;
      }
      mongo::BSONObj o = BSON("_id" << query_id << "user" << id_user_name.name);
      mongo::ScopedDbConnection c(config::get_mongodb_server());
      unsigned long long count = c->count(helpers::collection_name(Collections::QUERIES()), o );
      r = count > 0;
      c.done();
      return true;
    }

    bool set_bio_source_synonym(const std::string &bio_source_name, const std::string &synonymous,
                                bool is_bio_source, const bool is_syn, const std::string &user_key, std::string &msg)
    {
      if (!cv::set_bio_source_synonym(bio_source_name, synonymous, is_bio_source, is_syn, user_key, msg))  {
        return false;
      }
      return true;
    }

    bool get_bio_source_synonyms(const std::string &bio_source_name, const std::string &norm_bio_source_name,
                                 bool is_bio_source, const std::string &user_key,
                                 std::vector<utils::IdName> &syns,
                                 std::string &msg)
    {
      return cv::get_bio_source_synonyms(bio_source_name, norm_bio_source_name, is_bio_source, user_key, syns, msg);
    }

    bool set_bio_source_scope(const std::string &bio_source_more_embracing, const std::string &norm_bio_source_more_embracing,
                              const std::string &bio_source_less_embracing, const std::string &norm_bio_source_less_embracing,
                              bool more_embracing_is_syn, const bool less_embracing_is_syn,
                              const std::string &user_key, std::string &msg)
    {
      return cv::set_bio_source_embracing(bio_source_more_embracing, norm_bio_source_more_embracing,
                                          bio_source_less_embracing, norm_bio_source_less_embracing,
                                          more_embracing_is_syn, less_embracing_is_syn, user_key, msg);
    }

    bool get_bio_source_scope(const std::string &bio_source_name, const std::string &norm_bio_source_name,
                              bool is_bio_source, const std::string &user_key,
                              std::vector<utils::IdName> &related_bio_sources, std::string &msg)
    {
      std::vector<std::string> norm_subs;

      if (!cv::get_bio_source_embracing(bio_source_name, norm_bio_source_name, is_bio_source, user_key, norm_subs, msg)) {
        return false;
      }

      BOOST_FOREACH(std::string norm_sub, norm_subs) {
        utils::IdName sub_bio_source_name;
        if (!helpers::get_name(Collections::BIO_SOURCES(), norm_sub, sub_bio_source_name, msg)) {
          return false;
        }
        related_bio_sources.push_back(sub_bio_source_name);
      }
      return true;
    }

    bool get_user_name(const std::string &user_key, std::string &name, std::string &msg)
    {
      utils::IdName id_name;
      if (!helpers::get_name(Collections::USERS(), user_key, id_name, msg)) {
        return false;
      }
      name = id_name.name;
      return true;
    }

    bool get_user_name(const std::string &user_key, utils::IdName &id_name, std::string &msg)
    {
      return helpers::get_name(Collections::USERS(), user_key, id_name, msg);
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
                         std::string &annotation_id, std::string &msg)
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
        annotation_id = results[0]["_id"].str();
        return true;
      }

      std::vector<genomes::ChromosomeInfo> chromosomes;
      if (!get_chromosomes(norm_genome, chromosomes, msg)) {
        return false;
      }

      retrieve::SequenceRetriever retriever;
      std::vector<std::string> missing;
      BOOST_FOREACH(genomes::ChromosomeInfo chromosome_info, chromosomes) {
        if (!retriever.exists(genome, chromosome_info.name)) {
          missing.push_back(chromosome_info.name);
        }
      }
      if (!missing.empty()) {
        msg = "There is not sequence for the chromosomes '" + utils::vector_to_string(missing) + "'' of the genome " + genome + ". Please upload using 'upload_chromosome' command.";
      }

      ChromosomeRegionsList pattern_regions;
      BOOST_FOREACH(genomes::ChromosomeInfo chromosome_info, chromosomes) {
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
        ChromosomeRegions chromosome_regions(chromosome_info.name, regions);
        pattern_regions.push_back(chromosome_regions);
      }

      std::string description = "All localization of the pattern '" + pattern + "' in the genome " + genome;
      std::string norm_description = utils::normalize_name(description);

      Metadata extra_metadata;
      extra_metadata["pattern"] = pattern;
      if (overlap) {
        extra_metadata["overlap-style"] = "overlap";
      } else {
        extra_metadata["overlap-style"] = "non-overlap";
      }

      if (!insert_annotation(name, norm_name, genome, norm_genome, description, norm_description, extra_metadata,
                             user_key, ip, pattern_regions, parser::FileFormat::default_format(),
                             annotation_id, msg)) {
        return false;
      }

      return true;
    }

    bool find_annotation_pattern(const std::string &genome, const std::string &pattern, const bool overlap,
                                 std::string &annotation_id, std::string &msg)
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
      annotations_query_builder.append("done", true);

      mongo::ScopedDbConnection c(config::get_mongodb_server());

      mongo::BSONObj annotation_query = annotations_query_builder.obj();
      std::auto_ptr<mongo::DBClientCursor> cursor = c->query(helpers::collection_name(Collections::ANNOTATIONS()), annotation_query);

      if (cursor->more()) {
        mongo::BSONObj p = cursor->next();
        annotation_id = p.getField("_id").str();
        c.done();
        return true;
      } else {
        msg = "There is not pre-processed annotation for the";
        if (overlap) {
          msg = msg + " overlapped patterns.";
        } else {
          msg = msg + " non-overlapped patterns '";
        }
        msg = msg + pattern + "' for the genome '" + genome + "'.";
        c.done();
        return false;
      }
    }
  }
}
