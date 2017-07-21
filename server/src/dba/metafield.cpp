//
//  metafield.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 11.03.2014
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
#include <sstream>

#include <boost/bind.hpp>
#include <boost/ref.hpp>

#include <mongo/bson/bson.h>

#include "collections.hpp"
#include "helpers.hpp"
#include "key_mapper.hpp"
#include "metafield.hpp"
#include "queries.hpp"
#include "retrieve.hpp"

#include "../cache/column_dataset_cache.hpp"

#include "../connection/connection.hpp"

#include "../dba/genes.hpp"

#include "../processing/running_cache.hpp"

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
      m["@GENOME"] = &Metafield::genome;
      m["@SAMPLE_ID"] = &Metafield::sample_id;
      m["@STRAND"] = &Metafield::strand;
      m["@AGG.MIN"] = &Metafield::min;
      m["@AGG.MAX"] = &Metafield::max;
      m["@AGG.SUM"] = &Metafield::sum;
      m["@AGG.MEDIAN"] = &Metafield::median;
      m["@AGG.MEAN"] = &Metafield::mean;
      m["@AGG.VAR"] = &Metafield::var;
      m["@AGG.SD"] = &Metafield::sd;
      m["@AGG.COUNT"] = &Metafield::count;
      m["@COUNT.MOTIF"] = &Metafield::count_motif;
      m["@CALCULATED"] = &Metafield::calculated;
      m["@GENE_ATTRIBUTE"] = &Metafield::gene_attribute;
      m["@GENE_ID"] = &Metafield::gene_id;
      m["@GENE_NAME"] = &Metafield::gene_name;
      m["@GENE_EXPRESSION"] = &Metafield::gene_expression;
      m["@GO_IDS"] = &Metafield::go_ids;
      m["@GO_LABELS"] = &Metafield::go_labels;

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
      m["@GENOME"] = "string";
      m["@SAMPLE_ID"] = "string";
      m["@STRAND"] = "string";
      m["@AGG.MIN"] = "double";
      m["@AGG.MAX"] = "double";
      m["@AGG.SUM"] = "double";
      m["@AGG.MEDIAN"] = "double";
      m["@AGG.MEAN"] = "double";
      m["@AGG.VAR"] = "double";
      m["@AGG.SD"] = "double";
      m["@AGG.COUNT"] = "integer";
      m["@COUNT.MOTIF"] = "integer";
      m["@CALCULATED"] = "string";
      m["@GENE_ATTRIBUTE"] = "string";
      m["@GENE_ID"] = "string";
      m["@GENE_NAME"] = "string";
      m["@GENE_EXPRESSION"] = "double";
      m["@GO_IDS"] = "string";
      m["@GO_LABELS"] = "string";

      return m;
    }

    static const std::string EMPTY_STRING("");

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

    inline std::string metafield_attribute(const std::string& op)
    {
      unsigned int s = op.find("(") + 1;
      unsigned int e = op.find_last_of(")");
      unsigned int length = e - s;
      return op.substr(s, length);
    }

    bool Metafield::process(const std::string &op, const std::string &chrom, const AbstractRegion *region_ref,
                            processing::StatusPtr status, std::string &result, std::string &msg)
    {

      mongo::BSONObj obj;

      // TODO: Workaround - because aggregates does not have a region_set_id
      if (region_ref->dataset_id() != DATASET_EMPTY_ID) {
        if (!cache::get_bson_by_dataset_id(region_ref->dataset_id(), obj, msg)) {
          return false;
        }
      }

      static const std::string open_parenthesis("(");
      std::string command = op.substr(0, op.find(open_parenthesis));
      std::map<std::string, Function>::iterator it;
      it = functions.find(command);
      if (it != functions.end()) {
        Function f = it->second;
        return (*this.*f)(op, chrom, obj, region_ref, status, result, msg);
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
                           processing::StatusPtr status, std::string &result, std::string &msg)
    {

      result = utils::integer_to_string(region_ref->end() - region_ref->start());
      return true;
    }

    bool Metafield::strand(const std::string &op, const std::string &chrom, const mongo::BSONObj &obj, const AbstractRegion *region_ref,
                           processing::StatusPtr status, std::string &result, std::string &msg)
    {
      if (region_ref->has_strand()) {
        result = region_ref->strand();
      } else {
        int pos;
        if (!cache::get_column_position_from_dataset(region_ref->dataset_id(), "STRAND", pos, msg)) {
          return false;
        }
        if (pos == -1) {
          result = "";
        } else {
          result = region_ref->get_string(pos);
        }
      }
      return true;
    }

    bool Metafield::name(const std::string &op, const std::string &chrom, const mongo::BSONObj &obj, const AbstractRegion *region_ref,
                         processing::StatusPtr status, std::string &result, std::string &msg)
    {
      std::string name = get_by_region_set(obj, "name");
      result = name;
      return true;
    }

    bool Metafield::epigenetic_mark(const std::string &op, const std::string &chrom, const mongo::BSONObj &obj, const AbstractRegion *region_ref,
                                    processing::StatusPtr status,  std::string &result, std::string &msg)
    {
      result = get_by_region_set(obj, "epigenetic_mark");
      return true;
    }

    bool Metafield::project(const std::string &op, const std::string &chrom, const mongo::BSONObj &obj, const AbstractRegion *region_ref,
                            processing::StatusPtr status, std::string &result, std::string &msg)
    {
      result = get_by_region_set(obj, "project");
      return true;
    }

    bool Metafield::biosource(const std::string &op, const std::string &chrom, const mongo::BSONObj &obj, const AbstractRegion *region_ref,
                              processing::StatusPtr status, std::string &result, std::string &msg)
    {
      if (obj.hasField("sample_info")) {
        result =  obj["sample_info"]["biosource_name"].str();
      } else {
        result = "";
      }
      return true;
    }

    bool Metafield::genome(const std::string &op, const std::string &chrom, const mongo::BSONObj &obj, const AbstractRegion *region_ref,
                           processing::StatusPtr status, std::string &result, std::string &msg)
    {
      result = get_by_region_set(obj, "genome");
      return true;
    }

    bool Metafield::sample_id(const std::string &op, const std::string &chrom, const mongo::BSONObj &obj, const AbstractRegion *region_ref,
                              processing::StatusPtr status, std::string &result, std::string &msg)
    {
      result = get_by_region_set(obj, "sample_id");
      return true;
    }

    bool Metafield::sequence(const std::string &op, const std::string &chrom, const mongo::BSONObj &obj, const AbstractRegion *region_ref,
                             processing::StatusPtr status, std::string &result, std::string &msg)
    {
      std::string genome = get_by_region_set(obj, "norm_genome");

      if (!status->running_cache()->get_sequence(genome, chrom,
          region_ref->start(), region_ref->end(), result, status, msg)) {
        return false;
      }

      return true;
    }

    bool Metafield::count_motif(const std::string &op, const std::string &chrom, const mongo::BSONObj &obj, const AbstractRegion *region_ref,
                                processing::StatusPtr status, std::string &result, std::string &msg)
    {
      std::string pattern = metafield_attribute(op);

      std::string genome = get_by_region_set(obj, "genome");
      size_t count = 0;

      if (!status->running_cache()->count_regions(genome, chrom, pattern,
          region_ref->start(), region_ref->end(), count, status, msg)) {
        count = 0;
        return false;
      }

      result = utils::integer_to_string(count);
      return true;
    }


    bool Metafield::min(const std::string &op, const std::string &chrom, const mongo::BSONObj &obj, const AbstractRegion *region_ref,
                        processing::StatusPtr status, std::string &result, std::string &msg)
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
                        processing::StatusPtr status, std::string &result, std::string &msg)
    {
      if (region_ref->has_stats()) {
        const AggregateRegion *aggregate_region = static_cast<const AggregateRegion *>(region_ref);
        result = utils::score_to_string(aggregate_region->max());
      } else {
        result = "";
      }
      return true;
    }

    bool Metafield::sum(const std::string &op, const std::string &chrom, const mongo::BSONObj &obj, const AbstractRegion *region_ref,
                        processing::StatusPtr status, std::string &result, std::string &msg)
    {
      if (region_ref->has_stats()) {
        const AggregateRegion *aggregate_region = static_cast<const AggregateRegion *>(region_ref);
        result = utils::score_to_string(aggregate_region->sum());
      } else {
        result = "";
      }
      return true;
    }

    bool Metafield::median(const std::string &op, const std::string &chrom, const mongo::BSONObj &obj, const AbstractRegion *region_ref,
                           processing::StatusPtr status, std::string &result, std::string &msg)
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
                         processing::StatusPtr status, std::string &result, std::string &msg)
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
                        processing::StatusPtr status, std::string &result, std::string &msg)
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
                       processing::StatusPtr status, std::string &result, std::string &msg)
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
                          processing::StatusPtr status, std::string &result, std::string &msg)
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
                               processing::StatusPtr status, std::string &result, std::string &msg)
    {
      std::string code = metafield_attribute(op);

      lua::Sandbox::LuaPtr lua = lua::Sandbox::new_instance(status);
      if (!lua->store_row_code(code, msg)) {
        return false;
      }
      lua->set_current_context(chrom, region_ref, *this);

      return lua->execute_row_code(result, msg);
    }

    bool Metafield::gene_attribute(const std::string &op, const std::string &chrom, const mongo::BSONObj &obj, const AbstractRegion *region_ref,
                                   processing::StatusPtr status, std::string &result, std::string &msg)
    {
      std::string attribute_name = metafield_attribute(op);

      auto it = region_ref->attributes().find(attribute_name);
      if (it == region_ref->attributes().end()) {
        result = EMPTY_STRING;
        return true;
      }
      result = it->second;

      return true;
    }

    bool Metafield::gene_id(const std::string &op, const std::string &chrom, const mongo::BSONObj &obj, const AbstractRegion *region_ref,
                            processing::StatusPtr status, std::string &result, std::string &msg)
    {
      std::string strand;

      if (region_ref->has_strand()) {
        strand = region_ref->strand();
      } else {
        if (!Metafield::strand(op, chrom, obj, region_ref, status, strand, msg)) {
          return false;
        }
      }

      std::string gene_model = metafield_attribute(op);
      if (!dba::genes::get_gene_attribute(chrom, region_ref->start(), region_ref->end(), strand, "gene_id", gene_model, result, msg)) {
        return false;
      }
      return true;
    }

    bool Metafield::gene_name(const std::string &op, const std::string &chrom, const mongo::BSONObj &obj, const AbstractRegion *region_ref,
                              processing::StatusPtr status, std::string &result, std::string &msg)
    {
      std::string strand;

      if (region_ref->has_strand()) {
        strand = region_ref->strand();
      } else {
        if (!Metafield::strand(op, chrom, obj, region_ref, status, strand, msg)) {
          return false;
        }
      }

      std::string gene_model = metafield_attribute(op);
      if (!dba::genes::get_gene_attribute(chrom, region_ref->start(), region_ref->end(), strand, "gene_name",  gene_model, result, msg)) {
        return false;
      }
      return true;
    }

    bool Metafield::get_gene_ontology_terms(const std::string &op, const std::string &chrom, const mongo::BSONObj &obj, const AbstractRegion *region_ref,
                                            processing::StatusPtr status, std::vector<datatypes::GeneOntologyTermPtr>& go_terms, std::string &msg)
    {
      std::string strand;

      if (region_ref->has_strand()) {
        strand = region_ref->strand();
      } else {
        if (!Metafield::strand(op, chrom, obj, region_ref, status, strand, msg)) {
          return false;
        }
      }

      std::string gene_model = metafield_attribute(op);
      if (!dba::genes::get_gene_gene_ontology_annotations(chrom, region_ref->start(), region_ref->end(), strand, gene_model, go_terms, msg)) {
        return false;
      }

      return true;
    }


    bool Metafield::go_ids(const std::string &op, const std::string &chrom, const mongo::BSONObj &obj, const AbstractRegion *region_ref,
                           processing::StatusPtr status, std::string &result, std::string &msg)
    {
      std::vector<datatypes::GeneOntologyTermPtr> go_terms;
      if (!get_gene_ontology_terms(op, chrom, obj, region_ref, status, go_terms, msg)) {
        return false;
      }

      std::stringstream ss;
      bool first = true;
      for (const auto& go_term: go_terms) {
        if (!first) {
          ss << ",";
        }
        ss << go_term->go_id();
        first = false;
      }

      result = ss.str();

      return true;
    }

    bool Metafield::go_labels(const std::string &op, const std::string &chrom, const mongo::BSONObj &obj, const AbstractRegion *region_ref,
                              processing::StatusPtr status, std::string &result, std::string &msg)
    {
      std::vector<datatypes::GeneOntologyTermPtr> go_terms;
      if (!get_gene_ontology_terms(op, chrom, obj, region_ref, status, go_terms, msg)) {
        return false;
      }

      std::stringstream ss;
      bool first = true;
      for (const auto& go_term: go_terms) {
        if (!first) {
          ss << ",";
        }
        ss << go_term->go_label();
        first = false;
      }

      result = ss.str();

      return true;
    }

    bool Metafield::gene_expression(const std::string &op, const std::string &chrom, const mongo::BSONObj &obj, const AbstractRegion *region_ref,
                                    processing::StatusPtr status, std::string &result, std::string &msg)
    {
      return true;
    }
  }
}

std::map<std::string, epidb::dba::Metafield::Function> epidb::dba::Metafield::functions =  epidb::dba::Metafield::createFunctionsMap();

std::map<std::string, std::string> epidb::dba::Metafield::functionsReturns =  epidb::dba::Metafield::createFunctionsReturnsMap();
