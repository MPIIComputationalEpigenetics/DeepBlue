//
//  insert.cpp
//  epidb
//
//  Created by Felipe Albrecht on 30.04.14.
//  Copyright (c) 2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include <limits>
#include <map>
#include <string>
#include <sstream>
#include <vector>

#include <time.h>
#include <math.h>

#include <boost/foreach.hpp>

#include <mongo/bson/bson.h>
#include <mongo/client/dbclient.h>

#include "../extras/compress.hpp"
#include "../parser/field_type.hpp"
#include "../parser/genome_data.hpp"
#include "../parser/parser_factory.hpp"
#include "../extras/utils.hpp"
#include "../parser/wig.hpp"

#include "dba.hpp"
#include "collections.hpp"
#include "config.hpp"
#include "full_text.hpp"
#include "genomes.hpp"
#include "info.hpp"
#include "helpers.hpp"
#include "key_mapper.hpp"
#include "users.hpp"

#include "../regions.hpp"
#include "../log.hpp"


namespace epidb {
  namespace dba {

    const size_t BLOCK_SIZE = 100;
    const size_t BULK_SIZE = 10000;
    const size_t MAXIMUM_SIZE = 10000000; // from mongodb maximum message size: 48000000

    static const bool fill_region_builder(mongo::BSONObjBuilder &builder,
                                          const parser::Tokens &tokens, const parser::FileFormat &file_format,
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

      BOOST_FOREACH(const dba::columns::ColumnTypePtr & column_type, file_format) {
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
        BOOST_FOREACH(const std::string & s, tokens) {
          msg += s;
          msg += "\t";
        }
        return false;
      }

      if (!end_found) {
        msg = "End position was not informed or was ignored. Line content: ";
        BOOST_FOREACH(const std::string & s, tokens) {
          msg += s;
          msg += "\t";
        }
        return false;
      }

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


    // compress a block (vector of regions) and insert into blocks_bulk
    bool compress_block(const int dataset_id, const std::string &collection,
                        size_t &count,
                        std::vector<mongo::BSONObj> &block,
                        std::vector<mongo::BSONObj> &blocks_bulk, size_t &bulk_size, size_t &total_size)
    {
      mongo::BSONArrayBuilder ab;

      size_t features = 0;
      size_t uncompress_size = 0;
      int min = std::numeric_limits<int>::max();
      int max = std::numeric_limits<int>::min();
      for (std::vector<mongo::BSONObj>::const_iterator it = block.begin(); it != block.end(); it++) {
        uncompress_size += (*it).objsize();
        int start = (*it)[KeyMapper::START()].Int();
        int end = (*it)[KeyMapper::END()].Int();

        if (start < min) {
          min = start;
        }
        if (end > max) {
          max = end;
        }
        features++;
        ab.append(*it);
      }

      boost::shared_ptr<char> compressed_data;
      size_t compressed_size = 0;
      bool compressed = false;
      mongo::BSONObj o = ab.arr();
      compressed_data = epidb::compress::compress(o.objdata(), o.objsize(), compressed_size, compressed);

      mongo::BSONObjBuilder block_builder;

      long long id = (long long) dataset_id << 32 | (long long) count++ ;
      block_builder.append("_id", id);
      block_builder.append(KeyMapper::DATASET(), (int) dataset_id);

      block_builder.append(KeyMapper::START(), min);
      block_builder.append(KeyMapper::END(), max);

      if (compressed) {
        block_builder.append(KeyMapper::FEATURES(), (int) features);
        block_builder.append(KeyMapper::BED_COMPRESSED(), true);
        block_builder.append(KeyMapper::BED_DATASIZE(), o.objsize());
        block_builder.appendBinData(KeyMapper::BED_DATA(), compressed_size, mongo::BinDataGeneral, (void *) compressed_data.get());
      } else {
        block_builder.append(KeyMapper::FEATURES(), (int) features);
        block_builder.append(KeyMapper::BED_COMPRESSED(), false);
        block_builder.appendBinData(KeyMapper::BED_DATA(), o.objsize(), mongo::BinDataGeneral, o.objdata());
      }

      block.clear();
      mongo::BSONObj block_obj = block_builder.obj();
      blocks_bulk.push_back(block_obj);
      int block_size = block_obj.objsize();
      total_size += block_size;
      bulk_size += block_size;

      return true;
    }

    inline bool compress_and_insert_region_block(const int dataset_id,
        const std::string &collection, size_t &count,
        std::vector<mongo::BSONObj> &block, std::vector<mongo::BSONObj> &blocks_bulk,
        size_t &bulk_size, size_t &total_size,
        std::string &msg, size_t min_block_size = 1, size_t min_bulk_size = 1)
    {
      if (collection.empty()) {
        return true;
      }
      if (block.size() >= min_block_size) {
        compress_block(dataset_id, collection, count, block, blocks_bulk, bulk_size, total_size);
      }

      if (blocks_bulk.size() >= min_bulk_size || bulk_size >= MAXIMUM_SIZE) {
        mongo::ScopedDbConnection c(config::get_mongodb_server());
        c->insert(collection, blocks_bulk);
        if (!c->getLastError().empty()) {
          msg = c->getLastError();
          c.done();
          return false;
        }
        bulk_size = 0;
        blocks_bulk.clear();
        c.done();
      }

      return true;
    }

    // Verify if the collection (in this case, the chromosome) changed and applies changes.
    inline bool check_collection(const int dataset_id,
                                 const std::string &collection, std::string &prev_collection,
                                 size_t &count,
                                 std::vector<mongo::BSONObj> &block, std::vector<mongo::BSONObj> &blocks_bulk,
                                 size_t &bulk_size, size_t &total_size,
                                 std::string &msg)
    {
      if (prev_collection == collection) {
        return true;
      }

      if (!compress_and_insert_region_block(dataset_id, prev_collection, count, block, blocks_bulk, bulk_size, total_size, msg)) {
        return false;
      }

      prev_collection = collection;
      return true;
    }


    inline bool check_bulk_size(const int dataset_id, const std::string &collection, size_t &count,
                                std::vector<mongo::BSONObj> &block, std::vector<mongo::BSONObj> &blocks_bulk,
                                size_t &bulk_size, size_t &total_size,
                                std::string &msg)
    {
      if (!compress_and_insert_region_block(dataset_id, collection, count, block, blocks_bulk, bulk_size, total_size, msg, BLOCK_SIZE, BULK_SIZE)) {
        return false;
      }

      return true;
    }

    inline bool check_remainings(const int dataset_id, const std::string &collection, size_t &count,
                                 std::vector<mongo::BSONObj> &block, std::vector<mongo::BSONObj> &blocks_bulk,
                                 size_t &bulk_size, size_t &total_size,
                                 std::string &msg)
    {
      if (!compress_and_insert_region_block(dataset_id, collection, count, block, blocks_bulk, bulk_size, total_size, msg)) {
        return false;
      }

      return true;
    }


    bool build_experiment_metadata(const std::string &name, const std::string &norm_name,
                                   const std::string &genome, const std::string &norm_genome,
                                   const std::string &epigenetic_mark, const std::string &norm_epigenetic_mark,
                                   const std::string &sample_id, const std::string &technique, const std::string &norm_technique,
                                   const std::string &project, const std::string &norm_project,
                                   const std::string &description, const std::string &norm_description,
                                   const datatypes::Metadata &extra_metadata,
                                   const std::string &user_key, const std::string &ip,
                                   const parser::FileFormat &format,
                                   int &dataset_id,
                                   std::string &experiment_id,
                                   mongo::BSONObj &experiment_metadata,
                                   std::string &msg)
    {
      int e_id;
      if (!helpers::get_counter("experiments", e_id, msg))  {
        return false;
      }
      experiment_id = "e" + utils::integer_to_string(e_id);

      if (!helpers::get_counter("datasets", dataset_id, msg))  {
        return false;
      }

      mongo::BSONObjBuilder experiment_data_builder;
      experiment_data_builder.append("_id", experiment_id);
      experiment_data_builder.append(KeyMapper::DATASET(), dataset_id);
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

      experiment_data_builder.append("columns", format.to_bson());

      mongo::BSONObj extra_metadata_obj = datatypes::extra_metadata_to_bson(extra_metadata);
      experiment_data_builder.append("extra_metadata", extra_metadata_obj);

      std::map<std::string, std::string> sample_data;
      if (!info::get_sample_by_id(sample_id, sample_data, msg, true)) {
        return false;
      }
      mongo::BSONObjBuilder sample_builder;
      std::map<std::string, std::string>::iterator it;
      for (it = sample_data.begin(); it != sample_data.end(); ++it) {
        if ((it->first != "_id") && (it->first != "user")) {
          sample_builder.append(it->first, it->second);
        }
      }
      experiment_data_builder.append("sample_info", sample_builder.obj());

      experiment_metadata = experiment_data_builder.obj();

      return true;
    }


    bool build_annotation_info(const std::string &name, const std::string &norm_name,
                               const std::string &genome, const std::string &norm_genome,
                               const std::string &description, const std::string &norm_description,
                               const datatypes::Metadata &extra_metadata,
                               const std::string &user_key, const std::string &ip,
                               const parser::FileFormat &format,
                               int &dataset_id,
                               std::string &annotation_id,
                               mongo::BSONObj &annotation_metadata,
                               std::string &msg)
    {
      int a_id;
      if (!helpers::get_counter("annotations", a_id, msg))  {
        return false;
      }
      annotation_id = "a" + utils::integer_to_string(a_id);

      if (!helpers::get_counter("datasets", dataset_id, msg))  {
        return false;
      }

      mongo::BSONObjBuilder annotation_data_builder;
      annotation_data_builder.append("_id", annotation_id);
      annotation_data_builder.append(KeyMapper::DATASET(), dataset_id);
      annotation_data_builder.append("name", name);
      annotation_data_builder.append("norm_name", norm_name);
      annotation_data_builder.append("genome", genome);
      annotation_data_builder.append("norm_genome", norm_genome);
      annotation_data_builder.append("description", description);
      annotation_data_builder.append("norm_description", norm_description);
      annotation_data_builder.append("format", format.format());

      annotation_data_builder.append("columns", format.to_bson());

      mongo::BSONObj extra_metadata_obj = datatypes::extra_metadata_to_bson(extra_metadata);
      annotation_data_builder.append("extra_metadata", extra_metadata_obj);

      annotation_metadata = annotation_data_builder.obj();
      return true;
    }

    bool build_upload_info(const std::string &user_key, const std::string &client_address, const std::string &content_format,
                           mongo::BSONObj &upload_info, std::string &msg)
    {
      std::string user_name;
      if (!users::get_user_name(user_key, user_name, msg)) {
        return false;
      }

      mongo::BSONObjBuilder upload_info_builder;

      upload_info_builder.append("user", user_name);
      upload_info_builder.append("content_format", content_format);
      upload_info_builder.append("done", false);
      upload_info_builder.append("client_address", client_address);
      time_t time_;
      time(&time_);
      upload_info_builder.appendTimeT("upload_start", time_);

      upload_info = upload_info_builder.obj();

      return true;
    }

    bool update_upload_info(const std::string &collection, const std::string &annotation_id,
                            const int total_size, std::string &msg)
    {
      mongo::ScopedDbConnection c(config::get_mongodb_server());
      c->update(helpers::collection_name(collection), QUERY("_id" << annotation_id),
                BSON("$set" << BSON("upload_info.total_size" << (unsigned int) total_size << "upload_info.done" << true << "upload_info.upload_end" << mongo::DATENOW)), false, true);

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
                           const std::string &description, const std::string &norm_description,
                           const datatypes::Metadata &extra_metadata,
                           const std::string &user_key, const std::string &ip,
                           const parser::WigPtr &wig,
                           std::string &experiment_id, std::string &msg)
    {
      mongo::BSONObj experiment_metadata;
      int dataset_id;
      if (!build_experiment_metadata(name, norm_name, genome, norm_genome, epigenetic_mark, norm_epigenetic_mark,
                                     sample_id, technique, norm_technique, project, norm_project, description, norm_description, extra_metadata,
                                     user_key, ip, parser::FileFormat::wig_format(), dataset_id,  experiment_id, experiment_metadata, msg)) {
        return false;
      }

      mongo::BSONObj upload_info;
      if (!build_upload_info(user_key, ip, "wig", upload_info, msg)) {
        return false;
      }
      mongo::BSONObjBuilder experiment_builder;
      experiment_builder.appendElements(experiment_metadata);
      experiment_builder.append("upload_info", upload_info);

      mongo::BSONObj e = experiment_builder.obj();
      mongo::ScopedDbConnection c(config::get_mongodb_server());
      c->insert(helpers::collection_name(Collections::EXPERIMENTS()), e);
      if (!c->getLastError().empty()) {
        msg = c->getLastError();
        c.done();
        return false;
      }

      if (!search::insert_full_text(Collections::EXPERIMENTS(), experiment_id, experiment_metadata, msg)) {
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
      size_t actual_size = 0;
      size_t total_size = 0;
      size_t total_regions = 0;

      parser::WigContent::const_iterator end = wig->tracks_iterator_end();
      for (parser::WigContent::const_iterator it = wig->tracks_iterator(); it != end; it++) {
        parser::TrackPtr track = *it;
        mongo::BSONObjBuilder region_builder;
        region_builder.append("_id", (long long) dataset_id << 32 | (long long) count++);
        region_builder.append(KeyMapper::DATASET(), (int) dataset_id);

        region_builder.append(KeyMapper::WIG_TRACK_TYPE(), (int) track->type());
        region_builder.append(KeyMapper::START(), (int) track->start());
        region_builder.append(KeyMapper::END(), (int) track->end());
        if (track->step()) {
          region_builder.append(KeyMapper::WIG_STEP(), (int) track->step());
        }
        if (track->span()) {
          region_builder.append(KeyMapper::WIG_SPAN(), (int) track->span());
        }
        region_builder.append(KeyMapper::FEATURES(), (int) track->features());
        region_builder.append(KeyMapper::WIG_DATA_SIZE(), (int) track->data_size());

        boost::shared_ptr<char> data = track->data();
        size_t data_size = track->data_size();

        boost::shared_ptr<char> compressed_data;
        size_t compressed_size = 0;
        bool compressed = false;
        compressed_data = epidb::compress::compress(data.get(), data_size, compressed_size, compressed);

        if (compressed) {
          region_builder.append(KeyMapper::WIG_COMPRESSED(), true);
          region_builder.appendBinData(KeyMapper::WIG_DATA(), compressed_size, mongo::BinDataGeneral, (void *) compressed_data.get());
          actual_size += compressed_size;
        } else {
          region_builder.appendBinData(KeyMapper::WIG_DATA(), data_size, mongo::BinDataGeneral, data.get());
          actual_size += data_size;
        }

        total_regions +=  track->features();

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

        mongo::BSONObj r = region_builder.obj();
        std::string collection = helpers::region_collection_name(genome, internal_chromosome);

        if (prev_collection != collection) {
          if (!prev_collection.empty() && bulk.size() > 0) {
            c->insert(prev_collection, bulk);
            if (!c->getLastError().empty()) {
              msg = c->getLastError();
              c.done();
              return false;
            }
            bulk.clear();
          }
          prev_collection = collection;
          actual_size = 0;
        }

        total_size += r.objsize();
        bulk.push_back(r);

        if (bulk.size() % BULK_SIZE == 0 || actual_size > MAXIMUM_SIZE) {
          c->insert(collection, bulk);
          if (!c->getLastError().empty()) {
            msg = c->getLastError();
            c.done();
            return false;
          }
          bulk.clear();
          actual_size = 0;
        }
      }

      if (bulk.size() > 0) {
        c->insert(prev_collection, bulk);
        if (!c->getLastError().empty()) {
          msg = c->getLastError();
          c.done();
          return false;
        }
        bulk.clear();
      }

      if (!update_upload_info(Collections::EXPERIMENTS(), experiment_id, total_size, msg)) {
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
                           const std::string &description, const std::string &norm_description,
                           const datatypes::Metadata &extra_metadata,
                           const std::string &user_key, const std::string &ip,
                           const std::vector<parser::Tokens> &bed_file_tokenized,
                           const parser::FileFormat &format,
                           std::string &experiment_id, std::string &msg)
    {
      mongo::BSONObj experiment_metadata;
      int dataset_id;
      if (!build_experiment_metadata(name, norm_name, genome, norm_genome,
                                     epigenetic_mark, norm_epigenetic_mark,
                                     sample_id, technique, norm_technique, project, norm_project,
                                     description, norm_description, extra_metadata,
                                     user_key, ip, format,
                                     dataset_id,  experiment_id, experiment_metadata, msg)) {
        return false;
      }

      mongo::BSONObj upload_info;
      if (!build_upload_info(user_key, ip, "bed", upload_info, msg)) {
        return false;
      }

      mongo::BSONObjBuilder experiment_builder;
      experiment_builder.appendElements(experiment_metadata);
      experiment_builder.append("upload_info", upload_info);

      mongo::BSONObj e = experiment_builder.obj();

      mongo::ScopedDbConnection c(config::get_mongodb_server());
      c->insert(helpers::collection_name(Collections::EXPERIMENTS()), e);
      if (!c->getLastError().empty()) {
        msg = c->getLastError();
        c.done();
        return false;
      }

      if (!search::insert_full_text(Collections::EXPERIMENTS(), experiment_id, experiment_metadata, msg)) {
        c.done();
        return false;
      }

      genomes::GenomeInfoPtr genome_info;
      if (!genomes::get_genome_info(genome, genome_info, msg)) {
        c.done();
        return false;
      }

      size_t count = 0;
      size_t total_size = 0;
      size_t bulk_size = 0;
      std::vector<mongo::BSONObj> block;
      std::vector<mongo::BSONObj> blocks_bulk;
      std::string prev_collection;

      BOOST_FOREACH(const parser::Tokens & tokens, bed_file_tokenized) {
        mongo::BSONObjBuilder region_builder;
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
        std::string collection = helpers::region_collection_name(genome, internal_chromosome);

        if (!check_collection(dataset_id, collection, prev_collection, count, block, blocks_bulk, bulk_size, total_size, msg)) {
          c.done();
          return false;
        }

        block.push_back(r);

        if (!check_bulk_size(dataset_id, collection, count, block, blocks_bulk, bulk_size, total_size, msg)) {
          c.done();
          return false;
        }
      }

      if (!check_remainings(dataset_id, prev_collection, count, block, blocks_bulk, bulk_size, total_size, msg)) {
        c.done();
        return false;
      }

      if (!update_upload_info(Collections::EXPERIMENTS(), experiment_id, total_size, msg)) {
        return false;
      }

      c.done();
      return true;
    }

    bool insert_annotation(const std::string &name, const std::string &norm_name,
                           const std::string &genome, const std::string &norm_genome,
                           const std::string &description, const std::string &norm_description,
                           const datatypes::Metadata &extra_metadata,
                           const std::string &user_key, const std::string &ip,
                           const std::vector<parser::Tokens> &bed_file_tokenized,
                           const parser::FileFormat &format,
                           std::string &annotation_id, std::string &msg)
    {
      int dataset_id;
      mongo::BSONObj annotation_metadata;
      if (!build_annotation_info(name, norm_name, genome, norm_genome,
                                 description, norm_description, extra_metadata,
                                 user_key, ip, format,
                                 dataset_id, annotation_id, annotation_metadata, msg)) {
        return false;
      }

      mongo::BSONObj upload_info;
      if (!build_upload_info(user_key, ip, "internal", upload_info, msg)) {
        return false;
      }

      mongo::BSONObjBuilder annotation_data_builder;
      annotation_data_builder.appendElements(annotation_metadata);
      annotation_data_builder.append("upload_info", upload_info);

      mongo::BSONObj e = annotation_data_builder.obj();

      mongo::ScopedDbConnection c(config::get_mongodb_server());
      c->insert(helpers::collection_name(Collections::ANNOTATIONS()), e);
      if (!c->getLastError().empty()) {
        msg = c->getLastError();
        c.done();
        return false;
      }

      if (!search::insert_full_text(Collections::ANNOTATIONS(), annotation_id, annotation_metadata, msg)) {
        c.done();
        return false;
      }

      genomes::GenomeInfoPtr genome_info;
      if (!genomes::get_genome_info(genome, genome_info, msg)) {
        c.done();
        return false;
      }

      size_t count = 0;
      size_t total_size = 0;
      size_t bulk_size = 0;
      std::vector<mongo::BSONObj> block;
      std::vector<mongo::BSONObj> blocks_bulk;
      std::string prev_collection;

      BOOST_FOREACH(const parser::Tokens & tokens, bed_file_tokenized) {
        mongo::BSONObjBuilder region_builder;
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
        std::string collection = helpers::region_collection_name(genome, internal_chromosome);

        if (!check_collection(dataset_id, collection, prev_collection, count, block, blocks_bulk, bulk_size, total_size, msg)) {
          c.done();
          return false;
        }

        block.push_back(r);

        if (!check_bulk_size(dataset_id, collection, count, block, blocks_bulk, bulk_size, total_size, msg)) {
          c.done();
          return false;
        }
      }

      if (!check_remainings(dataset_id, prev_collection, count, block, blocks_bulk, bulk_size, total_size, msg)) {
        c.done();
        return false;
      }

      if (!update_upload_info(Collections::ANNOTATIONS(), annotation_id, total_size, msg)) {
        return false;
      }

      c.done();
      return true;
    }

    bool insert_annotation(const std::string &name, const std::string &norm_name,
                           const std::string &genome, const std::string &norm_genome,
                           const std::string &description, const std::string &norm_description,
                           const datatypes::Metadata &extra_metadata,
                           const std::string &user_key, const std::string &ip,
                           const ChromosomeRegionsList &regions,
                           const parser::FileFormat &format,
                           std::string &annotation_id, std::string &msg)
    {
      int dataset_id;
      mongo::BSONObj annotation_metadata;
      if (!build_annotation_info(name, norm_name, genome, norm_genome,
                                 description, norm_description, extra_metadata,
                                 user_key, ip, format,
                                 dataset_id, annotation_id, annotation_metadata, msg)) {
        return false;
      }

      mongo::BSONObj upload_info;
      if (!build_upload_info(user_key, ip, "internal", upload_info, msg)) {
        return false;
      }

      mongo::BSONObjBuilder annotation_data_builder;
      annotation_data_builder.appendElements(annotation_metadata);
      annotation_data_builder.append("upload_info", upload_info);

      mongo::BSONObj e = annotation_data_builder.obj();

      mongo::ScopedDbConnection c(config::get_mongodb_server());
      c->insert(helpers::collection_name(Collections::ANNOTATIONS()), e);
      if (!c->getLastError().empty()) {
        msg = c->getLastError();
        c.done();
        return false;
      }

      if (!search::insert_full_text(Collections::ANNOTATIONS(), annotation_id, annotation_metadata, msg)) {
        c.done();
        return false;
      }

      genomes::GenomeInfoPtr genome_info;
      if (!genomes::get_genome_info(genome, genome_info, msg)) {
        c.done();
        return false;
      }

      size_t count = 0;
      size_t total_size = 0;
      size_t bulk_size = 0;
      std::vector<mongo::BSONObj> block;
      std::vector<mongo::BSONObj> blocks_bulk;

      BOOST_FOREACH(const ChromosomeRegions & chromosome_regions, regions) {
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

        std::string collection = helpers::region_collection_name(genome, internal_chromosome);
        std::vector<mongo::BSONObj> bulk;

        for (RegionsIterator it = chromosome_regions.second->begin(); it != chromosome_regions.second->end(); it++) {
          const Region &region = *it;
          mongo::BSONObjBuilder region_builder;

          if (region.start() > chromosome_size || region.end() > chromosome_size) {
            msg = out_of_range_message(region.start(), region.end(), chromosome);
            c.done();
            return false;
          }

          region_builder.append(KeyMapper::START(), (int) region.start());
          region_builder.append(KeyMapper::END(), (int) region.end());

          mongo::BSONObj r = region_builder.obj();
          block.push_back(r);

          if (!check_bulk_size(dataset_id, collection, count, block, blocks_bulk, bulk_size, total_size, msg)) {
            c.done();
            return false;
          }
        }
        if (!check_remainings(dataset_id, collection, count, block, blocks_bulk, bulk_size, total_size, msg)) {
          c.done();
          return false;
        }
      }

      if (!update_upload_info(Collections::ANNOTATIONS(), annotation_id, total_size, msg)) {
        return false;
      }

      c.done();
      return true;
    }
  }
}
