//
//  insert.cpp
//  epidb
//
//  Created by Felipe Albrecht on 30.04.14.
//  Copyright (c) 2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include <map>
#include <string>
#include <sstream>
#include <vector>

#include <time.h>
#include <math.h>

#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread/mutex.hpp>

#include <mongo/bson/bson.h>
#include <mongo/client/dbclient.h>

#include "../extras/utils.hpp"
#include "../parser/field_type.hpp"
#include "../parser/genome_data.hpp"
#include "../parser/parser_factory.hpp"
#include "../parser/wig.hpp"

#include "dba.hpp"
#include "collections.hpp"
#include "config.hpp"
#include "controlled_vocabulary.hpp"
#include "full_text.hpp"
#include "genomes.hpp"
#include "info.hpp"
#include "helpers.hpp"
#include "key_mapper.hpp"

#include "../regions.hpp"
#include "../log.hpp"


namespace epidb {
  namespace dba {

    const size_t BULK_SIZE = 20000;


    static const bool fill_region_builder(mongo::BSONObjBuilder &builder, const parser::Tokens &tokens, const parser::FileFormat &file_format,
                                          std::string &chromosome, size_t &start, size_t &end, std::string &msg)
    {
      if (tokens.size() != file_format.size()) {
        msg = "size of tokens doesn't match size of file format.";
        return false;
      }

      size_t i(0);
      bool chr_found = false;
      bool start_found = false;
      bool end_found = false;

      BOOST_FOREACH(dba::columns::ColumnTypePtr column_type, file_format) {
        std::string field_name = column_type->name();
        std::string name;
        if (!KeyMapper::to_short(field_name, name, msg)) {
          return false;
        }
        parser::Token token = tokens[i++];

        if (!chr_found && field_name == "CHROMOSOME") {
          chr_found = true;
          chromosome = token;
          continue;
        }

        if (column_type->ignore(token)) {
          continue;
        }

        if (!column_type->check(token)) {
          msg = "Invalid value '" + token + "' for column " + field_name;
          return false;
        }

        if (column_type->type() == dba::columns::COLUMN_STRING) {
          builder.append(name, token);
        } else if (column_type->type() == dba::columns::COLUMN_INTEGER) {
          size_t l;
          if (!utils::string_to_long(token, l)) {
            msg = "The field '" + field_name + "' is an integer, but the value '" + token + "' is not a valid integer.";
            return false;
          }
          builder.append(name, (int) l);
        } else if (column_type->type() == dba::columns::COLUMN_DOUBLE) {
          double d;
          if (!utils::string_to_double(token, d)) {
            msg = "The field '" + field_name + "' is a double, but the value '" + token + "' is not a valid double.";
            return false;
          }
          builder.append(name, d);
        } else if (column_type->type() == dba::columns::COLUMN_CATEGORY) {
          builder.append(name, token);
        } else if (column_type->type() == dba::columns::COLUMN_RANGE) {
          double d;
          if (!utils::string_to_double(token, d)) {
            msg = "The field '" + field_name + "' is a double, but the value '" + token + "' is not a valid integer.";
            return false;
          }
          builder.append(name, d);
        } else {
          std::string err = "Invalid column type: " + column_type->str();
          EPIDB_LOG_ERR(err);
          msg = err;
          return false;
        }

        if (!start_found && field_name == "START") {
          start_found = true;
          size_t l;
          if (!utils::string_to_long(token, l)) {
            msg = "The field START should be an integer, but the value '" + token + "' is not a valid integer.";
            return false;
          }
          start = l;
        }

        if (!end_found && field_name == "END") {
          end_found = true;
          size_t l;
          if (!utils::string_to_long(token, l)) {
            msg = "The field END should be an integer, but the value '" + token + "' is not a valid integer.";
            return false;
          }
          end = l;
        }
      }

      if (!start_found) {
        msg = "Start position was not informed or was ignored. Line content: ";
        BOOST_FOREACH(std::string s, tokens) {
          msg += s;
          msg += "\t";
        }
        return false;
      }

      if (!end_found) {
        msg = "End position was not informed or was ignored. Line content: ";
        BOOST_FOREACH(std::string s, tokens) {
          msg += s;
          msg += "\t";
        }
        return false;
      }

      return true;
    }

    boost::mutex move_chunk_lock;
    bool set_index_shard(std::map<std::string, bool> &processed, const std::string &collection, const size_t chromosome_size,
                         std::string &msg)
    {
      if (processed.find(collection) != processed.end()) {
        return true;
      }

      EPIDB_LOG_DBG("Starting Indexing and Sharding for " << collection);

      mongo::ScopedDbConnection c(config::get_mongodb_server());
      mongo::BSONObj info;

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
      mongo::BSONObjBuilder index_rs;
      index_rs.append(KeyMapper::START(), 1);
      index_rs.append(KeyMapper::END(), 1);
      c->ensureIndex(collection, index_rs.obj());
      if (!c->getLastError().empty()) {
        msg = c->getLastError();
        EPIDB_LOG_ERR("Indexing on '" << collection << "' error: " << msg);
        c.done();
        return false;
      }

      if (config::sharding()) {
        EPIDB_LOG_DBG("Setting sharding for collection " << collection);

        mongo::BSONObjBuilder builder;
        builder.append("shardCollection", collection);
        builder.append("key", BSON(KeyMapper::START() << 1));
        mongo::BSONObj objShard = builder.obj();
        mongo::BSONObj info;
        EPIDB_LOG_DBG("Sharding: " << objShard.toString());

        if (!c->runCommand("admin", objShard, info)) {
          EPIDB_LOG_ERR("Sharding on '" << collection << "' error: " << info.toString());
          msg = "Error while sharding data. Check if you are connected to a MongoDB cluster with sharding enabled for the database '" + config::DATABASE_NAME() + "' or disable sharding: --nosharding";
          c.done();
          return false;
        }

        std::vector<std::string> shards_names = config::get_shards_names();

        static size_t i = 0;
        size_t actual_pos = i % shards_names.size();
        std::string shard = shards_names[actual_pos];
        i++;

        if (actual_pos != 0) { // if is not already the primary
          boost::mutex::scoped_lock lock(move_chunk_lock);
          EPIDB_LOG_DBG("Moving " << collection << " to " << shard);
          mongo::BSONObjBuilder move_builder;
          move_builder.append("moveChunk", collection);
          move_builder.append("find", BSON(KeyMapper::START() << 0)) ;
          move_builder.append("to", shard);

          mongo::BSONObj move = move_builder.obj();

          if (!c->runCommand("admin", move, info)) {
            EPIDB_LOG_WARN("Error while distributing the collection ("  << move.toString() << ") " <<  collection << " :" << info.toString());
            //msg = "(Internal Error) Error while distributing the data.";
            //c.done();
            //return false;
          }

        }

        EPIDB_LOG_DBG("Index and Sharding for " << collection << " done");
      }
      c.done();
      processed[collection] = true;

      return true;
    }

    std::string out_of_range_message(size_t start, size_t end, std::string &chrom )
    {
      std::stringstream ss;
      ss << "Invalid region: ";
      ss << start;
      ss << " - ";
      ss << end;
      ss << ". It is beyond the length of the chromosome ";
      ss << chrom;
      ss << " .";
      return ss.str();
    }



    bool insert_experiment(const std::string &name, const std::string &norm_name,
                           const std::string &genome, const std::string &norm_genome,
                           const std::string &epigenetic_mark, const std::string &norm_epigenetic_mark,
                           const std::string &sample_id, const std::string &technique, const std::string &norm_technique,
                           const std::string &project, const std::string &norm_project,
                           const std::string &description, const std::string &norm_description, const Metadata &extra_metadata,
                           const std::string &user_key, const std::string &ip, const parser::WigPtr &wig,
                           std::string &experiment_id, std::string &msg)
    {
      {
        int e_id;
        if (!helpers::get_counter("experiments", e_id, msg))  {
          return false;
        }
        experiment_id = "e" + boost::lexical_cast<std::string>(e_id);
      }

      mongo::BSONObjBuilder experiment_data_builder;
      experiment_data_builder.append("_id", experiment_id);
      experiment_data_builder.append("name", name);
      experiment_data_builder.append("norm_name", norm_name);
      experiment_data_builder.append("genome", genome);
      experiment_data_builder.append("norm_genome", norm_genome);
      experiment_data_builder.append("epigenetic_mark", epigenetic_mark);
      experiment_data_builder.append("norm_epigenetic_mark", norm_epigenetic_mark);
      experiment_data_builder.append("sample_id", sample_id);
      experiment_data_builder.append("technique", technique);
      experiment_data_builder.append("norm_technique", norm_technique);
      experiment_data_builder.append("project", project);
      experiment_data_builder.append("norm_project", norm_project);
      experiment_data_builder.append("description", description);
      experiment_data_builder.append("norm_description", norm_description);
      experiment_data_builder.append("content_format", "wig");


      mongo::BSONObjBuilder metadata_builder;
      Metadata::const_iterator cit;
      for (cit = extra_metadata.begin(); cit != extra_metadata.end(); ++cit) {
        metadata_builder.append(cit->first, cit->second);
      }
      experiment_data_builder.append("extra_metadata", metadata_builder.obj());


      std::map<std::string, std::string> sample_data;
      if (!info::get_sample_by_id(sample_id, sample_data, msg, true)) {
        return false;
      }
      std::map<std::string, std::string>::iterator it;
      for (it = sample_data.begin(); it != sample_data.end(); ++it) {
        if ((it->first != "_id") && (it->first != "user")) {
          experiment_data_builder.append(it->first, it->second);
        }
      }

      mongo::BSONObj experiment_data = experiment_data_builder.obj();
      mongo::BSONObjBuilder experiment_builder;
      experiment_builder.appendElements(experiment_data);

      std::string user_name;
      if (!get_user_name(user_key, user_name, msg)) {
        return false;
      }
      experiment_builder.append("user", user_name);
      experiment_builder.append("done", false);
      experiment_builder.append("client_address", ip);
      time_t time_;
      time(&time_);
      experiment_builder.appendTimeT("upload_start", time_);

      mongo::BSONObj e = experiment_builder.obj();

      mongo::ScopedDbConnection c(config::get_mongodb_server());
      c->insert(helpers::collection_name(Collections::EXPERIMENTS()), e);
      if (!c->getLastError().empty()) {
        msg = c->getLastError();
        c.done();
        return false;
      }

      if (!search::insert_full_text(Collections::EXPERIMENTS(), experiment_id, experiment_data, msg)) {
        c.done();
        return false;
      }

      genomes::GenomeInfoPtr genome_info;
      if (!genomes::get_genome_info(genome, genome_info, msg)) {
        c.done();
        return false;
      }

      size_t count = 0;
      std::vector<mongo::BSONObj> bulk;
      std::string prev_collection;
      size_t prev_size;
      std::map<std::string, bool> processed;

      parser::WigContent::const_iterator end = wig->tracks_iterator_end();
      for (parser::WigContent::const_iterator it = wig->tracks_iterator(); it != end; it++) {
        parser::TrackPtr track = *it;
        mongo::BSONObjBuilder region_builder;
        region_builder.append("_id", (int) count++);

        if (track->type() == parser::FIXED_STEP) {
          region_builder.append(KeyMapper::WIG_TRACK_TYPE(), "F");
        } else {
          region_builder.append(KeyMapper::WIG_TRACK_TYPE(), "V");
        }

        region_builder.append(KeyMapper::START(), (int) track->start());
        region_builder.append(KeyMapper::WIG_STEP(), (int) (int) track->step());
        region_builder.append(KeyMapper::START(), (int) track->start());
        region_builder.append(KeyMapper::END(), (int) track->end());
        region_builder.append(KeyMapper::WIG_SPAN(), (int) track->span());
        region_builder.append(KeyMapper::WIG_FEATURES(), (int) track->features());
        region_builder.append(KeyMapper::WIG_DATA_SIZE(), (int) track->data_size());
        region_builder.appendBinData(KeyMapper::WIG_DATA(), track->data_size(), mongo::BinDataGeneral, track->data());

        std::string internal_chromosome;
        if (!genome_info->internal_chromosome(track->chromosome(), internal_chromosome, msg)) {
          // TODO: delete data already included.
          c.done();
          return false;
        }

        size_t size;
        if (!genome_info->chromosome_size(internal_chromosome, size, msg)) {
          // TODO: delete data already included.
          c.done();
          return false;
        }

        /*
        if (feature._start > size || feature._end > size) {
          msg = out_of_range_message(feature._start, feature._end, feature._chrom);
          c.done();
          return false;
        }
        */

        mongo::BSONObj r = region_builder.obj();
        std::string collection = helpers::region_collection_name(genome, experiment_id, internal_chromosome);

        if (prev_collection != collection) {
          if (!prev_collection.empty() && bulk.size() > 0) {
            if (!set_index_shard(processed, prev_collection, prev_size, msg)) {
              c.done();
              return false;
            }
            c->insert(prev_collection, bulk);
            if (!c->getLastError().empty()) {
              msg = c->getLastError();
              c.done();
              return false;
            }
            bulk.clear();
          }
          prev_collection = collection;
          prev_size = size;
        }

        bulk.push_back(r);

        if (bulk.size() % BULK_SIZE == 0) {
          if (!set_index_shard(processed, collection, size, msg)) {
            c.done();
            return false;
          }
          c->insert(collection, bulk);
          if (!c->getLastError().empty()) {
            msg = c->getLastError();
            c.done();
            return false;
          }
          bulk.clear();
        }
      }

      if (bulk.size() > 0) {
        if (!set_index_shard(processed, prev_collection, prev_size, msg)) {
          c.done();
          return false;
        }
        c->insert(prev_collection, bulk);
        if (!c->getLastError().empty()) {
          msg = c->getLastError();
          c.done();
          return false;
        }
        bulk.clear();
      }

      c->update(helpers::collection_name(Collections::EXPERIMENTS()), QUERY("_id" << experiment_id),
                BSON("$set" << BSON("done" << true << "upload_end" << mongo::DATENOW)), false, true);

      if (!c->getLastError().empty()) {
        msg = c->getLastError();
        c.done();
        return false;
      }

      c.done();
      return true;
    }

    bool insert_experiment(const std::string &name, const std::string &norm_name,
                           const std::string &genome, const std::string &norm_genome,
                           const std::string &epigenetic_mark, const std::string &norm_epigenetic_mark,
                           const std::string &sample_id, const std::string &technique, const std::string &norm_technique,
                           const std::string &project, const std::string &norm_project,
                           const std::string &description, const std::string &norm_description, const Metadata &extra_metadata,
                           const std::string &user_key, const std::string &ip,
                           const std::vector<parser::Tokens> &bed_file_tokenized,
                           const parser::FileFormat &format,
                           std::string &experiment_id, std::string &msg)
    {
      {
        int e_id;
        if (!helpers::get_counter("experiments", e_id, msg))  {
          return false;
        }
        experiment_id = "e" + boost::lexical_cast<std::string>(e_id);
      }

      mongo::BSONObjBuilder experiment_data_builder;
      experiment_data_builder.append("_id", experiment_id);
      experiment_data_builder.append("name", name);
      experiment_data_builder.append("norm_name", norm_name);
      experiment_data_builder.append("genome", genome);
      experiment_data_builder.append("norm_genome", genome);
      experiment_data_builder.append("epigenetic_mark", epigenetic_mark);
      experiment_data_builder.append("norm_epigenetic_mark", norm_epigenetic_mark);
      experiment_data_builder.append("sample_id", sample_id);
      experiment_data_builder.append("technique", technique);
      experiment_data_builder.append("norm_technique", norm_technique);
      experiment_data_builder.append("project", project);
      experiment_data_builder.append("norm_project", norm_project);
      experiment_data_builder.append("description", description);
      experiment_data_builder.append("norm_description", norm_description);
      experiment_data_builder.append("format", format.format());
      experiment_data_builder.append("content_format", "bed");

      mongo::BSONObjBuilder metadata_builder;
      Metadata::const_iterator cit;
      for (cit = extra_metadata.begin(); cit != extra_metadata.end(); ++cit) {
        metadata_builder.append(cit->first, cit->second);
      }
      experiment_data_builder.append("extra_metadata", metadata_builder.obj());

      std::map<std::string, std::string> sample_data;
      if (!info::get_sample_by_id(sample_id, sample_data, msg, true)) {
        return false;
      }
      std::map<std::string, std::string>::iterator it;
      for (it = sample_data.begin(); it != sample_data.end(); ++it) {
        if ((it->first != "_id") && (it->first != "user")) {
          experiment_data_builder.append(it->first, it->second);
        }
      }

      mongo::BSONObjBuilder experiment_builder;
      mongo::BSONObj experiment_data = experiment_data_builder.obj();
      experiment_builder.appendElements(experiment_data);

      std::string user_name;
      if (!get_user_name(user_key, user_name, msg)) {
        return false;
      }
      experiment_builder.append("user", user_name);
      experiment_builder.append("done", false);
      experiment_builder.append("client_address", ip);
      time_t time_;
      time(&time_);
      experiment_builder.appendTimeT("upload_start", time_);
      mongo::BSONObj e = experiment_builder.obj();

      mongo::ScopedDbConnection c(config::get_mongodb_server());
      c->insert(helpers::collection_name(Collections::EXPERIMENTS()), e);
      if (!c->getLastError().empty()) {
        msg = c->getLastError();
        c.done();
        return false;
      }

      if (!search::insert_full_text(Collections::EXPERIMENTS(), experiment_id, experiment_data, msg)) {
        c.done();
        return false;
      }

      genomes::GenomeInfoPtr genome_info;
      if (!genomes::get_genome_info(genome, genome_info, msg)) {
        c.done();
        return false;
      }

      size_t count = 0;
      std::vector<mongo::BSONObj> bulk;
      std::string prev_collection;
      size_t prev_size;
      std::map<std::string, bool> processed;
      BOOST_FOREACH( parser::Tokens tokens, bed_file_tokenized) {
        mongo::BSONObjBuilder region_builder;
        region_builder.append("_id", (int) count++);
        std::string chromosome;
        size_t start;
        size_t end;
        if (!fill_region_builder(region_builder, tokens, format, chromosome, start, end, msg)) {
          c.done();
          return false;
        }
        std::string internal_chromosome;
        if (!genome_info->internal_chromosome(chromosome, internal_chromosome, msg)) {
          // TODO: delete data already included.
          c.done();
          return false;
        }
        size_t size;
        if (!genome_info->chromosome_size(internal_chromosome, size, msg)) {
          // TODO: delete data already included.
          c.done();
          return false;
        }

        if (start > size || end > size) {
          msg = out_of_range_message(start, end, chromosome);
          c.done();
          return false;
        }

        mongo::BSONObj r = region_builder.obj();
        std::string collection = helpers::region_collection_name(genome, experiment_id, internal_chromosome);

        if (prev_collection != collection) {
          if (!prev_collection.empty() && bulk.size() > 0) {
            if (!set_index_shard(processed, prev_collection, prev_size, msg)) {
              c.done();
              return false;
            }
            c->insert(prev_collection, bulk);
            if (!c->getLastError().empty()) {
              msg = c->getLastError();
              c.done();
              return false;
            }
            bulk.clear();
          }
          prev_collection = collection;
          prev_size = size;
        }

        bulk.push_back(r);

        if (bulk.size() % BULK_SIZE == 0) {
          if (!set_index_shard(processed, collection, size, msg)) {
            c.done();
            return false;
          }
          c->insert(collection, bulk);
          if (!c->getLastError().empty()) {
            msg = c->getLastError();
            c.done();
            return false;
          }
          bulk.clear();
        }
      }

      if (bulk.size() > 0) {
        if (!set_index_shard(processed, prev_collection, prev_size, msg)) {
          c.done();
          return false;
        }
        c->insert(prev_collection, bulk);
        if (!c->getLastError().empty()) {
          msg = c->getLastError();
          c.done();
          return false;
        }
        bulk.clear();
      }

      c->update(helpers::collection_name(Collections::EXPERIMENTS()), QUERY("_id" << experiment_id),
                BSON("$set" << BSON("done" << true << "upload_end" << mongo::DATENOW)), false, true);

      if (!c->getLastError().empty()) {
        msg = c->getLastError();
        c.done();
        return false;
      }

      c.done();
      return true;
    }

    bool insert_annotation(const std::string &name, const std::string &norm_name,
                           const std::string &genome, const std::string &norm_genome,
                           const std::string &description, const std::string &norm_description, const Metadata &extra_metadata,
                           const std::string &user_key, const std::string &ip,
                           const std::vector<parser::Tokens> &bed_file_tokenized,
                           const parser::FileFormat &format,
                           std::string &annotation_id, std::string &msg)
    {
      {
        int a_id;
        if (!helpers::get_counter("annotations", a_id, msg))  {
          return false;
        }
        annotation_id = "a" + boost::lexical_cast<std::string>(a_id);
      }

      mongo::ScopedDbConnection c(config::get_mongodb_server());
      mongo::BSONObjBuilder annotation_data_builder;
      annotation_data_builder.append("_id", annotation_id);
      annotation_data_builder.append("name", name);
      annotation_data_builder.append("norm_name", norm_name);
      annotation_data_builder.append("genome", genome);
      annotation_data_builder.append("norm_genome", norm_genome);
      annotation_data_builder.append("description", description);
      annotation_data_builder.append("norm_description", norm_description);
      annotation_data_builder.append("format", format.format());

      mongo::BSONObjBuilder metadata_builder;
      Metadata::const_iterator cit;
      for (cit = extra_metadata.begin(); cit != extra_metadata.end(); ++cit) {
        metadata_builder.append(cit->first, cit->second);
      }
      annotation_data_builder.append("extra_metadata", metadata_builder.obj());

      // TODO: extra metadata
      mongo::BSONObj search_data = annotation_data_builder.obj();
      mongo::BSONObjBuilder annotation_builder;
      annotation_builder.appendElements(search_data);

      std::string user_name;
      if (!get_user_name(user_key, user_name, msg)) {
        c.done();
        return false;
      }
      annotation_builder.append("user", user_name);
      annotation_builder.append("done", false);
      annotation_builder.append("client_address", ip);
      time_t time_;
      time(&time_);
      annotation_builder.appendTimeT("upload_start", time_);
      // TODO: internal metadata:
      // - extra_columns
      mongo::BSONObj a = annotation_builder.obj();
      c->insert(helpers::collection_name(Collections::ANNOTATIONS()), a);
      if (!c->getLastError().empty()) {
        msg = c->getLastError();
        c.done();
        return false;
      }

      if (!search::insert_full_text(Collections::ANNOTATIONS(), annotation_id, search_data, msg)) {
        c.done();
        return false;
      }

      genomes::GenomeInfoPtr genome_info;
      if (!genomes::get_genome_info(genome, genome_info, msg)) {
        c.done();
        return false;
      }

      size_t count = 0;
      std::vector<mongo::BSONObj> bulk;
      std::string prev_collection;
      size_t prev_size;
      std::map<std::string, bool> processed;
      BOOST_FOREACH( parser::Tokens tokens, bed_file_tokenized) {
        mongo::BSONObjBuilder region_builder;
        region_builder.append("_id", (int) count++);
        std::string chromosome;
        size_t start;
        size_t end;

        if (!fill_region_builder(region_builder, tokens, format, chromosome, start, end, msg)) {
          c.done();
          return false;
        }
        std::string internal_chromosome;
        if (!genome_info->internal_chromosome(chromosome, internal_chromosome, msg)) {
          // TODO: delete data already included.
          c.done();
          return false;
        }
        size_t size;
        if (!genome_info->chromosome_size(internal_chromosome, size, msg)) {
          // TODO: delete data already included.
          c.done();
          return false;
        }

        if (start > size || end > size) {
          msg = out_of_range_message(start, end, chromosome);
          c.done();
          return false;
        }
        mongo::BSONObj r = region_builder.obj();
        std::string collection = helpers::region_collection_name(genome, annotation_id, internal_chromosome);
        if (prev_collection != collection) {
          if (!prev_collection.empty() && bulk.size() > 0) {
            if (!set_index_shard(processed, prev_collection, prev_size, msg)) {
              c.done();
              return false;
            }
            c->insert(prev_collection, bulk);
            if (!c->getLastError().empty()) {
              msg = c->getLastError();
              c.done();
              return false;
            }
            bulk.clear();
          }
          prev_collection = collection;
          prev_size = size;
        }

        bulk.push_back(r);

        if (bulk.size() % BULK_SIZE == 0) {
          if (!set_index_shard(processed, collection, size, msg)) {
            c.done();
            return false;
          }
          c->insert(collection, bulk);
          if (!c->getLastError().empty()) {
            msg = c->getLastError();
            c.done();
            return false;
          }
          bulk.clear();
        }
      }

      if (bulk.size() > 0) {
        if (!set_index_shard(processed, prev_collection, prev_size, msg)) {
          c.done();
          return false;
        }
        c->insert(prev_collection, bulk);
        if (!c->getLastError().empty()) {
          msg = c->getLastError();
          c.done();
          return false;
        }
        bulk.clear();
      }

      c->update(helpers::collection_name(Collections::ANNOTATIONS()), QUERY("_id" << annotation_id),
                BSON("$set" << BSON("done" << true << "upload_end" << mongo::DATENOW)), false, true);

      if (!c->getLastError().empty()) {
        msg = c->getLastError();
        c.done();
        return false;
      }

      c.done();
      return true;
    }

    bool insert_annotation(const std::string &name, const std::string &norm_name,
                           const std::string &genome, const std::string &norm_genome,
                           const std::string &description, const std::string &norm_description, const Metadata &extra_metadata,
                           const std::string &user_key, const std::string &ip,
                           const ChromosomeRegionsList &regions,
                           const parser::FileFormat &format,
                           std::string &annotation_id, std::string &msg)
    {
      {
        int a_id;
        if (!helpers::get_counter("annotations", a_id, msg))  {
          return false;
        }
        annotation_id = "a" + boost::lexical_cast<std::string>(a_id);
      }

      mongo::ScopedDbConnection c(config::get_mongodb_server());
      mongo::BSONObjBuilder annotation_data_builder;
      annotation_data_builder.append("_id", annotation_id);
      annotation_data_builder.append("name", name);
      annotation_data_builder.append("norm_name", norm_name);
      annotation_data_builder.append("genome", genome);
      annotation_data_builder.append("norm_genome", norm_genome);
      annotation_data_builder.append("description", description);
      annotation_data_builder.append("norm_description", norm_description);
      annotation_data_builder.append("format", format.format());

      mongo::BSONObjBuilder metadata_builder;
      Metadata::const_iterator cit;
      for (cit = extra_metadata.begin(); cit != extra_metadata.end(); ++cit) {
        metadata_builder.append(cit->first, cit->second);
      }
      annotation_data_builder.append("extra_metadata", metadata_builder.obj());

      // TODO: extra metadata
      mongo::BSONObj search_data = annotation_data_builder.obj();
      mongo::BSONObjBuilder annotation_builder;
      annotation_builder.appendElements(search_data);

      std::string user_name;
      if (!get_user_name(user_key, user_name, msg)) {
        c.done();
        return false;
      }
      annotation_builder.append("user", user_name);
      annotation_builder.append("done", false);
      annotation_builder.append("client_address", ip);
      time_t time_;
      time(&time_);
      annotation_builder.appendTimeT("upload_start", time_);
      // TODO: internal metadata:
      // - extra_columns
      mongo::BSONObj a = annotation_builder.obj();
      c->insert(helpers::collection_name(Collections::ANNOTATIONS()), a);
      if (!c->getLastError().empty()) {
        msg = c->getLastError();
        c.done();
        return false;
      }

      if (!search::insert_full_text(Collections::ANNOTATIONS(), annotation_id, search_data, msg)) {
        c.done();
        return false;
      }

      genomes::GenomeInfoPtr genome_info;
      if (!genomes::get_genome_info(genome, genome_info, msg)) {
        c.done();
        return false;
      }

      size_t count = 0;
      std::map<std::string, bool> processed;
      BOOST_FOREACH(ChromosomeRegions chromosome_regions, regions) {
        std::string chromosome = chromosome_regions.first;
        std::string internal_chromosome;
        size_t chromosome_size;

        if (!genome_info->internal_chromosome(chromosome, internal_chromosome, msg)) {
          // TODO: delete data already included.
          c.done();
          return false;
        }

        if (!genome_info->chromosome_size(internal_chromosome, chromosome_size, msg)) {
          // TODO: delete data already included.
          c.done();
          return false;
        }

        std::string collection = helpers::region_collection_name(genome, annotation_id, internal_chromosome);
        std::vector<mongo::BSONObj> bulk;

        if (!set_index_shard(processed, collection, chromosome_size, msg)) {
          c.done();
          return false;
        }

        for (RegionsIterator it = chromosome_regions.second->begin(); it != chromosome_regions.second->end(); it++) {
          Region region = *it;
          mongo::BSONObjBuilder region_builder;
          region_builder.append("_id", (int) count++);

          if (region.start() > chromosome_size || region.end() > chromosome_size) {
            msg = out_of_range_message(region.start(), region.end(), chromosome);
            c.done();
            return false;
          }

          region_builder.append(KeyMapper::START(), (int) region.start());
          region_builder.append(KeyMapper::END(), (int) region.end());

          mongo::BSONObj r = region_builder.obj();
          bulk.push_back(r);

          if (bulk.size() % BULK_SIZE == 0) {
            c->insert(collection, bulk);
            if (!c->getLastError().empty()) {
              msg = c->getLastError();
              c.done();
              return false;
            }
            bulk.clear();
          }
        }
        if (bulk.size() > 0) {
          c->insert(collection, bulk);
          if (!c->getLastError().empty()) {
            msg = c->getLastError();
            c.done();
            return false;
          }
          bulk.clear();
        }
      }

      c->update(helpers::collection_name(Collections::ANNOTATIONS()), QUERY("_id" << annotation_id),
                BSON("$set" << BSON("done" << true << "upload_end" << mongo::DATENOW)), false, true);

      if (!c->getLastError().empty()) {
        msg = c->getLastError();
        c.done();
        return false;
      }

      c.done();
      return true;
    }
  }
}
