//
//  metafield.hpp
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

#ifndef EPIDB_DBA_METAFIELD_HPP
#define EPIDB_DBA_METAFIELD_HPP

#include <map>

#include <mongo/bson/bson.h>

#include "../datatypes/regions.hpp"

#include "../dba/retrieve.hpp"

namespace epidb {
  namespace dba {
    class Metafield {

    private:
      typedef bool (Metafield::*Function)(const std::string &, const std::string &, const mongo::BSONObj &, const AbstractRegion *, processing::StatusPtr, std::string &, std::string &);

      static std::map<std::string, Function> functions;
      static std::map<std::string, std::string> functionsReturns;

      std::map<DatasetId, mongo::BSONObj> obj_by_dataset_id;

      static const std::map<std::string, Function> createFunctionsMap();

      static const std::map<std::string, std::string> createFunctionsReturnsMap();

      bool length(const std::string &, const std::string &, const mongo::BSONObj &obj, const AbstractRegion *region_ref, processing::StatusPtr status, std::string &result, std::string &msg);

      bool name(const std::string &, const std::string &, const mongo::BSONObj &obj, const AbstractRegion *region_ref, processing::StatusPtr status, std::string &result, std::string &msg);

      bool strand(const std::string &, const std::string &, const mongo::BSONObj &obj, const AbstractRegion *region_ref, processing::StatusPtr status, std::string &result, std::string &msg);

      bool sequence(const std::string &, const std::string &, const mongo::BSONObj &obj, const AbstractRegion *region_ref, processing::StatusPtr status, std::string &result, std::string &msg);

      bool count_motif(const std::string &, const std::string &, const mongo::BSONObj &obj, const AbstractRegion *region_ref, processing::StatusPtr status, std::string &result, std::string &msg);

      bool epigenetic_mark(const std::string &op, const std::string &chrom, const mongo::BSONObj &obj, const AbstractRegion *region_ref, processing::StatusPtr status, std::string &result, std::string &msg);

      bool calculated(const std::string &op, const std::string &chrom, const mongo::BSONObj &obj, const AbstractRegion *region_ref, processing::StatusPtr status, std::string &result, std::string &msg);

      bool project(const std::string &op, const std::string &chrom, const mongo::BSONObj &obj, const AbstractRegion *region_ref, processing::StatusPtr status, std::string &result, std::string &msg);

      bool biosource(const std::string &op, const std::string &chrom, const mongo::BSONObj &obj, const AbstractRegion *region_ref, processing::StatusPtr status, std::string &result, std::string &msg);

      bool genome(const std::string &op, const std::string &chrom, const mongo::BSONObj &obj, const AbstractRegion *region_ref, processing::StatusPtr status, std::string &result, std::string &msg);

      bool sample_id(const std::string &op, const std::string &chrom, const mongo::BSONObj &obj, const AbstractRegion *region_ref, processing::StatusPtr status, std::string &result, std::string &msg);

      bool min(const std::string &op, const std::string &chrom, const mongo::BSONObj &obj, const AbstractRegion *region_ref, processing::StatusPtr status, std::string &result, std::string &msg);

      bool max(const std::string &op, const std::string &chrom, const mongo::BSONObj &obj, const AbstractRegion *region_ref, processing::StatusPtr status, std::string &result, std::string &msg);

      bool sum(const std::string &op, const std::string &chrom, const mongo::BSONObj &obj, const AbstractRegion *region_ref, processing::StatusPtr status, std::string &result, std::string &msg);

      bool median(const std::string &op, const std::string &chrom, const mongo::BSONObj &obj, const AbstractRegion *region_ref, processing::StatusPtr status, std::string &result, std::string &msg);

      bool mean(const std::string &op, const std::string &chrom, const mongo::BSONObj &obj, const AbstractRegion *region_ref, processing::StatusPtr status, std::string &result, std::string &msg);

      bool var(const std::string &op, const std::string &chrom, const mongo::BSONObj &obj, const AbstractRegion *region_ref, processing::StatusPtr status, std::string &result, std::string &msg);

      bool sd(const std::string &op, const std::string &chrom, const mongo::BSONObj &obj, const AbstractRegion *region_ref, processing::StatusPtr status, std::string &result, std::string &msg);

      bool count(const std::string &op, const std::string &chrom, const mongo::BSONObj &obj, const AbstractRegion *region_ref, processing::StatusPtr status, std::string &result, std::string &msg);

      bool gene_attribute(const std::string &op, const std::string &chrom, const mongo::BSONObj &obj, const AbstractRegion *region_ref, processing::StatusPtr status, std::string &result, std::string &msg);

      bool gene_id(const std::string &op, const std::string &chrom, const mongo::BSONObj &obj, const AbstractRegion *region_ref, processing::StatusPtr status, std::string &result, std::string &msg);

      bool gene_name(const std::string &op, const std::string &chrom, const mongo::BSONObj &obj, const AbstractRegion *region_ref, processing::StatusPtr status, std::string &result, std::string &msg);

      bool gene_expression(const std::string &op, const std::string &chrom, const mongo::BSONObj &obj, const AbstractRegion *region_ref, processing::StatusPtr status, std::string &result, std::string &msg);

      bool get_gene_ontology_terms(const std::string &op, const std::string &chrom, const mongo::BSONObj &obj, const AbstractRegion *region_ref, processing::StatusPtr status, std::vector<datatypes::GeneOntologyTermPtr>& go_terms, std::string &msg);

      bool go_ids(const std::string &op, const std::string &chrom, const mongo::BSONObj &obj, const AbstractRegion *region_ref, processing::StatusPtr status, std::string &result, std::string &msg);

      bool go_labels(const std::string &op, const std::string &chrom, const mongo::BSONObj &obj, const AbstractRegion *region_ref, processing::StatusPtr status, std::string &result, std::string &msg);

    public:
      static bool is_meta(const std::string &s);
      static std::string command_type(const std::string &command);

      bool process(const std::string &, const std::string &, const AbstractRegion *region, processing::StatusPtr status, std::string &result, std::string &msg);
    };
  }
}

#endif
