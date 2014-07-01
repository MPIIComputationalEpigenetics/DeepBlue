//
//  metafield.cpp
//  epidb
//
//  Created by Felipe Albrecht on 11.03.2014
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include <map>

#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/ref.hpp>

#include <mongo/bson/bson.h>
#include <mongo/client/dbclient.h>

#include "config.hpp"
#include "collections.hpp"
#include "helpers.hpp"
#include "key_mapper.hpp"
#include "metafield.hpp"
#include "queries.hpp"
#include "retrieve.hpp"

#include "../log.hpp"
#include "../extras/utils.hpp"

namespace epidb {
  namespace dba {

    const std::map<const std::string, Metafield::Function> Metafield::createFunctionsMap()
    {
      std::map<const std::string, Metafield::Function> m;
      m["@LENGTH"] = &Metafield::length;
      m["@NAME"] = &Metafield::name;
      m["@SEQUENCE"] = &Metafield::sequence;
      m["@EPIGENETIC_MARK"] = &Metafield::epigenetic_mark;
      m["@PROJECT"] = &Metafield::project;
      m["@BIO_SOURCE"] = &Metafield::bio_source;
      m["@SAMPLE_ID"] = &Metafield::sample_id;
      m["@AGG.MIN"] = &Metafield::min;
      m["@AGG.MAX"] = &Metafield::max;
      m["@AGG.MEDIAN"] = &Metafield::median;
      m["@AGG.MEAN"] = &Metafield::mean;
      m["@AGG.VAR"] = &Metafield::var;
      m["@AGG.SD"] = &Metafield::sd;
      m["@AGG.COUNT"] = &Metafield::count;
      m["@COUNT.OVERLAP"] = &Metafield::count_overlap;
      m["@COUNT.NON-OVERLAP"] = &Metafield::count_non_overlap;

      return m;
    }

    bool Metafield::is_meta(const std::string &s)
    {
      return (s[0] == '@' || s[0] == '$');
    }

    bool Metafield::get_bson_by_collection_id(CollectionId collection_id, mongo::BSONObj &obj, std::string &msg)
    {

      if (obj_by_collection_id.find(collection_id) != obj_by_collection_id.end()) {
        obj = obj_by_collection_id.find(collection_id)->second;
        return true;
      }

      mongo::ScopedDbConnection c(config::get_mongodb_server());

      std::string id = *collection_id;

      std::auto_ptr<mongo::DBClientCursor> data_cursor;
      if (id[0] == 'e') {
        data_cursor = c->query(helpers::collection_name(Collections::EXPERIMENTS()),
                               mongo::Query(BSON("_id" << id)));
      }

      else if (id[0] == 'a') {
        data_cursor = c->query(helpers::collection_name(Collections::ANNOTATIONS()),
                               mongo::Query(BSON("_id" << id)));

      } if (id[0] == 't') {
        data_cursor = c->query(helpers::collection_name(Collections::TILINGS()),
                               mongo::Query(BSON("_id" << id)));
      }

      if (data_cursor->more()) {
        obj = data_cursor->next().getOwned();
        obj_by_collection_id[collection_id] = obj;
        c.done();
        return true;
      }

      c.done();
      msg = "source with id " + id + " not found.";
      return false;
    }

    bool Metafield::process(const std::string &op, const std::string &chrom, const Region &region,
                            std::string &result, std::string &msg)
    {

      mongo::BSONObj obj;

      // TODO: Workaround - because aggregates does not have a region_set_id
      if (region.collection_id() != EMPTY_COLLECTION_ID) {
        if (!get_bson_by_collection_id(region.collection_id(), obj, msg)) {
          return false;
        }
      }

      std::string command = op.substr(0, op.find('('));
      std::map<const std::string, Function>::iterator it;
      it = functions.find(command);
      if (it != functions.end()) {
        Function f = it->second;
        return (*this.*f)(op, chrom, obj, region, result, msg);
      }
      msg = "Metafield " + op + " does not exist.";
      return false;
    }

    const std::string get_by_region_set(const mongo::BSONObj &obj, const std::string &field)
    {
      if (!obj.hasField(field)) {
        return "";
      }
      return obj.getField(field).str();
    }

    bool Metafield::length(const std::string &op, const std::string &chrom, const mongo::BSONObj &obj, const Region &region,
                           std::string &result, std::string &msg)
    {

      result = boost::lexical_cast<std::string>(region.end() - region.start());
      return true;
    }

    bool Metafield::name(const std::string &op, const std::string &chrom, const mongo::BSONObj &obj, const Region &region,
                         std::string &result, std::string &msg)
    {
      result = get_by_region_set(obj, "name");
      return true;
    }

    bool Metafield::epigenetic_mark(const std::string &op, const std::string &chrom, const mongo::BSONObj &obj, const Region &region,
                                    std::string &result, std::string &msg)
    {
      result = get_by_region_set(obj, "epigenetic_mark");
      return true;
    }

    bool Metafield::project(const std::string &op, const std::string &chrom, const mongo::BSONObj &obj, const Region &region,
                            std::string &result, std::string &msg)
    {
      result = get_by_region_set(obj, "project");
      return true;
    }

    bool Metafield::bio_source(const std::string &op, const std::string &chrom, const mongo::BSONObj &obj, const Region &region,
                               std::string &result, std::string &msg)
    {
      result = get_by_region_set(obj, "bio_source_name");
      return true;
    }

    bool Metafield::sample_id(const std::string &op, const std::string &chrom, const mongo::BSONObj &obj, const Region &region,
                              std::string &result, std::string &msg)
    {
      result = get_by_region_set(obj, "sample_id");
      return true;
    }

    bool Metafield::sequence(const std::string &op, const std::string &chrom, const mongo::BSONObj &obj, const Region &region,
                             std::string &result, std::string &msg)
    {

      std::string genome = get_by_region_set(obj, "genome");

      std::string sequence;
      if (seq_retr.exists(genome, chrom)) {
        if (!seq_retr.retrieve(genome, chrom, region.start(), region.end(), sequence, msg)) {
          return false;
        }
        result = sequence;
      } else {
        result = "";
      }
      return true;
    }

    bool Metafield::count_pattern(const std::string &pattern, const std::string &genome, const std::string &chrom,
                                  const Region &region, const bool overlap,
                                  size_t &count, std::string &msg)
    {
      std::string annotation_id;
      if (!dba::find_annotation_pattern(genome, pattern, overlap, annotation_id, msg)) {
        count = 0;
        return false;
      }

      mongo::BSONObjBuilder region_query_builder;

      region_query_builder.append(KeyMapper::START(), BSON("$gte" << (int) region.start() << "$lte" << (int) region.end()));
      region_query_builder.append(KeyMapper::END(), BSON("$gte" << (int) region.start() << "$lte" << (int) region.end()));

      mongo::BSONObj region_query = region_query_builder.obj();

      if (!retrieve::count_regions(genome, annotation_id, chrom, region_query, count)) {
        msg = "Error while counting regions for " + genome + " " + chrom + " " + region_query.toString();
        count = 0;
        return false;
      }

      return true;
    }

    bool Metafield::count_overlap(const std::string &op, const std::string &chrom, const mongo::BSONObj &obj, const Region &region,
                                  std::string &result, std::string &msg)
    {
      unsigned int s = op.find("(") + 1;
      unsigned int e = op.find_last_of(")");
      unsigned int length = e - s;

      std::string pattern = op.substr(s, length);

      std::string genome = get_by_region_set(obj, "genome");
      size_t count = 0;
      if (!count_pattern(pattern, genome, chrom, region, true, count, msg)) {
        return false;
      }

      result = boost::lexical_cast<std::string>(count);
      return true;
    }

    bool Metafield::count_non_overlap(const std::string &op, const std::string &chrom, const mongo::BSONObj &obj, const Region &region,
                                      std::string &result, std::string &msg)
    {
      unsigned int s = op.find("(") + 1;
      unsigned int e = op.find_last_of(")");
      unsigned int length = e - s;

      std::string pattern = op.substr(s, length);

      std::string genome = get_by_region_set(obj, "genome");
      size_t count = 0;
      if (!count_pattern(pattern, genome, chrom, region, false, count, msg)) {
        return false;
      }
      result = boost::lexical_cast<std::string>(count);

      return true;
    }


    bool Metafield::min(const std::string &op, const std::string &chrom, const mongo::BSONObj &obj, const Region &region,
                        std::string &result, std::string &msg)
    {
      if (region.has_stats()) {
        result = utils::double_to_string(region.min());
      } else {
        result = "";
      }
      return true;
    }

    bool Metafield::max(const std::string &op, const std::string &chrom, const mongo::BSONObj &obj, const Region &region,
                        std::string &result, std::string &msg)
    {
      if (region.has_stats()) {
        result = utils::double_to_string(region.max());
      } else {
        result = "";
      }
      return true;
    }

    bool Metafield::median(const std::string &op, const std::string &chrom, const mongo::BSONObj &obj, const Region &region,
                           std::string &result, std::string &msg)
    {
      if (region.has_stats()) {
        result = utils::double_to_string(region.median());
      } else {
        result = "";
      }
      return true;
    }

    bool Metafield::mean(const std::string &op, const std::string &chrom, const mongo::BSONObj &obj, const Region &region,
                         std::string &result, std::string &msg)
    {
      if (region.has_stats()) {
        result = utils::double_to_string(region.mean());
      } else {
        result = "";
      }
      return true;
    }


    bool Metafield::var(const std::string &op, const std::string &chrom, const mongo::BSONObj &obj, const Region &region,
                        std::string &result, std::string &msg)
    {
      if (region.has_stats()) {
        result = utils::double_to_string(region.var());
      } else {
        result = "";
      }
      return true;
    }

    bool Metafield::sd(const std::string &op, const std::string &chrom, const mongo::BSONObj &obj, const Region &region,
                       std::string &result, std::string &msg)
    {
      if (region.has_stats()) {
        result = utils::double_to_string(region.sd());
      } else {
        result = "";
      }
      return true;
    }

    bool Metafield::count(const std::string &op, const std::string &chrom, const mongo::BSONObj &obj, const Region &region,
                          std::string &result, std::string &msg)
    {
      if (region.has_stats()) {
        result = boost::lexical_cast<std::string>(region.count());
      } else {
        result = "";
      }
      return true;
    }
  }
}

std::map<const std::string, epidb::dba::Metafield::Function> epidb::dba::Metafield::functions =  epidb::dba::Metafield::createFunctionsMap();