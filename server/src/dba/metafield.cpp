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
#include <boost/ref.hpp>

#include <mongo/bson/bson.h>

#include "collections.hpp"
#include "connection.hpp"
#include "helpers.hpp"
#include "key_mapper.hpp"
#include "metafield.hpp"
#include "queries.hpp"
#include "retrieve.hpp"

#include "../lua/sandbox.hpp"

#include "../errors.hpp"
#include "../log.hpp"
#include "../extras/utils.hpp"

namespace epidb {
  namespace dba {

    const std::map<std::string, Metafield::Function> Metafield::createFunctionsMap()
    {
      std::map<std::string, Metafield::Function> m;
      m["@LENGTH"] = &Metafield::length;
      m["@NAME"] = &Metafield::name;
      m["@SEQUENCE"] = &Metafield::sequence;
      m["@EPIGENETIC_MARK"] = &Metafield::epigenetic_mark;
      m["@PROJECT"] = &Metafield::project;
      m["@BIOSOURCE"] = &Metafield::biosource;
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
      m["@CALCULATED"] = &Metafield::calculated;

      return m;
    }

    const std::map<std::string, std::string> Metafield::createFunctionsReturnsMap()
    {
      std::map<std::string, std::string> m;
      m["@LENGTH"] = "integer";
      m["@NAME"] = "string";
      m["@SEQUENCE"] = "string";
      m["@EPIGENETIC_MARK"] = "string";
      m["@PROJECT"] = "string";
      m["@BIOSOURCE"] = "string";
      m["@SAMPLE_ID"] = "string";
      m["@AGG.MIN"] = "double";
      m["@AGG.MAX"] = "double";
      m["@AGG.MEDIAN"] = "double";
      m["@AGG.MEAN"] = "double";
      m["@AGG.VAR"] = "double";
      m["@AGG.SD"] = "double";
      m["@AGG.COUNT"] = "integer";
      m["@COUNT.OVERLAP"] = "integer";
      m["@COUNT.NON-OVERLAP"] = "integer";
      m["@CALCULATED"] = "string";

      return m;
    }

    std::string Metafield::command_type(const std::string &command)
    {
      std::map<std::string, std::string>::iterator it = functionsReturns.find(command);
      if (it == functionsReturns.end()) {
        return std::string();
      }
      return it->second;
    }

    bool Metafield::is_meta(const std::string &s)
    {
      return (s[0] == '@' || s[0] == '$');
    }

    bool Metafield::get_bson_by_dataset_id(DatasetId dataset_id, mongo::BSONObj &obj, std::string &msg)
    {

      if (obj_by_dataset_id.find(dataset_id) != obj_by_dataset_id.end()) {
        obj = obj_by_dataset_id.find(dataset_id)->second;
        return true;
      }

      Connection c;

      std::auto_ptr<mongo::DBClientCursor> data_cursor;
      data_cursor = c->query(helpers::collection_name(Collections::EXPERIMENTS()),
                             mongo::Query(BSON(KeyMapper::DATASET() << dataset_id)));
      if (data_cursor->more()) {
        obj = data_cursor->next().getOwned();
        obj_by_dataset_id[dataset_id] = obj;
        c.done();
        return true;
      }

      data_cursor = c->query(helpers::collection_name(Collections::ANNOTATIONS()),
                             mongo::Query(BSON(KeyMapper::DATASET() << dataset_id)));
      if (data_cursor->more()) {
        obj = data_cursor->next().getOwned();
        obj_by_dataset_id[dataset_id] = obj;
        c.done();
        return true;
      }


      data_cursor = c->query(helpers::collection_name(Collections::TILINGS()),
                             mongo::Query(BSON(KeyMapper::DATASET() << dataset_id)));
      if (data_cursor->more()) {
        obj = data_cursor->next().getOwned();
        obj_by_dataset_id[dataset_id] = obj;
        c.done();
        return true;
      }

      c.done();
      msg = Error::m(ERR_DATASET_NOT_FOUND, dataset_id);
      return false;
    }

    bool Metafield::process(const std::string &op, const std::string &chrom, const AbstractRegion *region_ref,
                            std::string &result, std::string &msg)
    {

      mongo::BSONObj obj;

      // TODO: Workaround - because aggregates does not have a region_set_id
      if (region_ref->dataset_id() != DATASET_EMPTY_ID) {
        if (!get_bson_by_dataset_id(region_ref->dataset_id(), obj, msg)) {
          return false;
        }
      }

      static const std::string open_parenthesis("(");
      std::string command = op.substr(0, op.find(open_parenthesis));
      std::map<std::string, Function>::iterator it;
      it = functions.find(command);
      if (it != functions.end()) {
        Function f = it->second;
        return (*this.*f)(op, chrom, obj, region_ref, result, msg);
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

    bool Metafield::length(const std::string &op, const std::string &chrom, const mongo::BSONObj &obj, const AbstractRegion *region_ref,
                           std::string &result, std::string &msg)
    {

      result = utils::integer_to_string(region_ref->end() - region_ref->start());
      return true;
    }

    bool Metafield::name(const std::string &op, const std::string &chrom, const mongo::BSONObj &obj, const AbstractRegion *region_ref,
                         std::string &result, std::string &msg)
    {
      result = get_by_region_set(obj, "name");
      return true;
    }

    bool Metafield::epigenetic_mark(const std::string &op, const std::string &chrom, const mongo::BSONObj &obj, const AbstractRegion *region_ref,
                                    std::string &result, std::string &msg)
    {
      result = get_by_region_set(obj, "epigenetic_mark");
      return true;
    }

    bool Metafield::project(const std::string &op, const std::string &chrom, const mongo::BSONObj &obj, const AbstractRegion *region_ref,
                            std::string &result, std::string &msg)
    {
      result = get_by_region_set(obj, "project");
      return true;
    }

    bool Metafield::biosource(const std::string &op, const std::string &chrom, const mongo::BSONObj &obj, const AbstractRegion *region_ref,
                              std::string &result, std::string &msg)
    {
      if (obj.hasField("sample_info")) {
        result =  obj["sample_info"]["biosource_name"].str();
      } else {
        result = "";
      }
      return true;
    }

    bool Metafield::sample_id(const std::string &op, const std::string &chrom, const mongo::BSONObj &obj, const AbstractRegion *region_ref,
                              std::string &result, std::string &msg)
    {
      result = get_by_region_set(obj, "sample_id");
      return true;
    }

    bool Metafield::sequence(const std::string &op, const std::string &chrom, const mongo::BSONObj &obj, const AbstractRegion *region_ref,
                             std::string &result, std::string &msg)
    {

      std::string genome = get_by_region_set(obj, "genome");

      std::string sequence;
      if (seq_retr.exists(genome, chrom)) {
        if (!seq_retr.retrieve(genome, chrom, region_ref->start(), region_ref->end(), sequence, msg)) {
          return false;
        }
        result = sequence;
      } else {
        result = "";
      }
      return true;
    }

    bool Metafield::count_pattern(const std::string &pattern, const std::string &genome, const std::string &chrom,
                                  const AbstractRegion *region_ref, const bool overlap,
                                  size_t &count, std::string &msg)
    {
      DatasetId dataset_id;
      if (!dba::find_annotation_pattern(genome, pattern, overlap, dataset_id, msg)) {
        count = 0;
        return false;
      }

      mongo::BSONObjBuilder region_query_builder;

      region_query_builder.append(KeyMapper::DATASET(), dataset_id);
      region_query_builder.append(KeyMapper::START(), BSON("$gte" << (int) region_ref->start() << "$lte" << (int) region_ref->end()));
      region_query_builder.append(KeyMapper::END(), BSON("$gte" << (int) region_ref->start() << "$lte" << (int) region_ref->end()));

      mongo::BSONObj region_query = region_query_builder.obj();

      if (!retrieve::count_regions(genome, chrom, region_query, count)) {
        msg = "Error while counting regions for " + genome + " " + chrom + " " + region_query.toString();
        count = 0;
        return false;
      }

      return true;
    }

    bool Metafield::count_overlap(const std::string &op, const std::string &chrom, const mongo::BSONObj &obj, const AbstractRegion *region_ref,
                                  std::string &result, std::string &msg)
    {
      unsigned int s = op.find("(") + 1;
      unsigned int e = op.find_last_of(")");
      unsigned int length = e - s;

      std::string pattern = op.substr(s, length);

      std::string genome = get_by_region_set(obj, "genome");
      size_t count = 0;
      if (!count_pattern(pattern, genome, chrom, region_ref, true, count, msg)) {
        return false;
      }

      result = utils::integer_to_string(count);
      return true;
    }

    bool Metafield::count_non_overlap(const std::string &op, const std::string &chrom, const mongo::BSONObj &obj, const AbstractRegion *region_ref,
                                      std::string &result, std::string &msg)
    {
      unsigned int s = op.find("(") + 1;
      unsigned int e = op.find_last_of(")");
      unsigned int length = e - s;

      std::string pattern = op.substr(s, length);

      std::string genome = get_by_region_set(obj, "genome");
      size_t count = 0;
      if (!count_pattern(pattern, genome, chrom, region_ref, false, count, msg)) {
        return false;
      }
      result = utils::integer_to_string(count);

      return true;
    }


    bool Metafield::min(const std::string &op, const std::string &chrom, const mongo::BSONObj &obj, const AbstractRegion *region_ref,
                        std::string &result, std::string &msg)
    {
      if (region_ref->has_stats()) {
        const AggregateRegion *aggregate_region = static_cast<const AggregateRegion *>(region_ref);
        result = utils::score_to_string(aggregate_region->min());
      } else {
        result = "";
      }
      return true;
    }

    bool Metafield::max(const std::string &op, const std::string &chrom, const mongo::BSONObj &obj, const AbstractRegion *region_ref,
                        std::string &result, std::string &msg)
    {
      if (region_ref->has_stats()) {
        const AggregateRegion *aggregate_region = static_cast<const AggregateRegion *>(region_ref);
        result = utils::score_to_string(aggregate_region->max());
      } else {
        result = "";
      }
      return true;
    }

    bool Metafield::median(const std::string &op, const std::string &chrom, const mongo::BSONObj &obj, const AbstractRegion *region_ref,
                           std::string &result, std::string &msg)
    {
      if (region_ref->has_stats()) {
        const AggregateRegion *aggregate_region = static_cast<const AggregateRegion *>(region_ref);
        result = utils::score_to_string(aggregate_region->median());
      } else {
        result = "";
      }
      return true;
    }

    bool Metafield::mean(const std::string &op, const std::string &chrom, const mongo::BSONObj &obj, const AbstractRegion *region_ref,
                         std::string &result, std::string &msg)
    {
      if (region_ref->has_stats()) {
        const AggregateRegion *aggregate_region = static_cast<const AggregateRegion *>(region_ref);
        result = utils::score_to_string(aggregate_region->mean());
      } else {
        result = "";
      }
      return true;
    }


    bool Metafield::var(const std::string &op, const std::string &chrom, const mongo::BSONObj &obj, const AbstractRegion *region_ref,
                        std::string &result, std::string &msg)
    {
      if (region_ref->has_stats()) {
        const AggregateRegion *aggregate_region = static_cast<const AggregateRegion *>(region_ref);
        result = utils::score_to_string(aggregate_region->var());
      } else {
        result = "";
      }
      return true;
    }

    bool Metafield::sd(const std::string &op, const std::string &chrom, const mongo::BSONObj &obj, const AbstractRegion *region_ref,
                       std::string &result, std::string &msg)
    {
      if (region_ref->has_stats()) {
        const AggregateRegion *aggregate_region = static_cast<const AggregateRegion *>(region_ref);
        result = utils::score_to_string(aggregate_region->sd());
      } else {
        result = "";
      }
      return true;
    }

    bool Metafield::count(const std::string &op, const std::string &chrom, const mongo::BSONObj &obj, const AbstractRegion *region_ref,
                          std::string &result, std::string &msg)
    {
      if (region_ref->has_stats()) {
        const AggregateRegion *aggregate_region = static_cast<const AggregateRegion *>(region_ref);
        result = utils::integer_to_string(aggregate_region->count());
      } else {
        result = "";
      }
      return true;
    }

    bool Metafield::calculated(const std::string &op, const std::string &chrom, const mongo::BSONObj &obj, const AbstractRegion *region_ref,
                               std::string &result, std::string &msg)
    {
      unsigned int s = op.find("(") + 1;
      unsigned int e = op.find_last_of(")");
      unsigned int length = e - s;

      std::string code = op.substr(s, length);

      lua::Sandbox::LuaPtr lua = lua::Sandbox::new_instance();
      if (!lua->store_row_code(code, msg)) {
        return false;
      }
      lua->set_current_context(chrom, region_ref, *this);

      return lua->execute_row_code(result, msg);
    }
  }
}

std::map<std::string, epidb::dba::Metafield::Function> epidb::dba::Metafield::functions =  epidb::dba::Metafield::createFunctionsMap();

std::map<std::string, std::string> epidb::dba::Metafield::functionsReturns =  epidb::dba::Metafield::createFunctionsReturnsMap();
