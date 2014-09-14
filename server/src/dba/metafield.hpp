//
//  metafield.hpp
//  epidb
//
//  Created by Felipe Albrecht on 11.03.2014
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#ifndef EPIDB_DBA_METAFIELD_HPP
#define EPIDB_DBA_METAFIELD_HPP

#include <map>

#include <mongo/bson/bson.h>

#include <boost/enable_shared_from_this.hpp>

#include "../dba/column_types.hpp"
#include "../dba/retrieve.hpp"

#include "../regions.hpp"

namespace epidb {
  namespace dba {
    class Metafield {

    private:
      typedef bool (Metafield::*Function)(const std::string &, const std::string &, const mongo::BSONObj &, const Region &, std::string &, std::string &);

      static std::map<std::string, Function> functions;
      static std::map<std::string, std::string> functionsReturns;

      std::unordered_map<DatasetId, mongo::BSONObj> obj_by_dataset_id;

      dba::retrieve::SequenceRetriever seq_retr;

      static const std::map<std::string, Function> createFunctionsMap();

      static const std::map<std::string, std::string> createFunctionsReturnsMap();

      bool get_bson_by_dataset_id(DatasetId dataset_id, mongo::BSONObj &obj, std::string &msg);

      bool count_pattern(const std::string &pattern, const std::string &genome, const std::string &chrom,
                         const Region &region, const bool overlap,
                         size_t &count, std::string &msg);

      bool length(const std::string &, const std::string &, const mongo::BSONObj &obj, const Region &region, std::string &result, std::string &msg);

      bool name(const std::string &, const std::string &, const mongo::BSONObj &obj, const Region &region, std::string &result, std::string &msg);

      bool sequence(const std::string &, const std::string &, const mongo::BSONObj &obj, const Region &region, std::string &result, std::string &msg);

      bool count_overlap(const std::string &, const std::string &, const mongo::BSONObj &obj, const Region &region, std::string &result, std::string &msg);

      bool count_non_overlap(const std::string &, const std::string &, const mongo::BSONObj &obj, const Region &region, std::string &result, std::string &msg);

      bool epigenetic_mark(const std::string &op, const std::string &chrom, const mongo::BSONObj &obj, const Region &region, std::string &result, std::string &msg);

      bool project(const std::string &op, const std::string &chrom, const mongo::BSONObj &obj, const Region &region, std::string &result, std::string &msg);

      bool bio_source(const std::string &op, const std::string &chrom, const mongo::BSONObj &obj, const Region &region, std::string &result, std::string &msg);

      bool sample_id(const std::string &op, const std::string &chrom, const mongo::BSONObj &obj, const Region &region, std::string &result, std::string &msg);

      bool min(const std::string &op, const std::string &chrom, const mongo::BSONObj &obj, const Region &region, std::string &result, std::string &msg);

      bool max(const std::string &op, const std::string &chrom, const mongo::BSONObj &obj, const Region &region, std::string &result, std::string &msg);

      bool median(const std::string &op, const std::string &chrom, const mongo::BSONObj &obj, const Region &region, std::string &result, std::string &msg);

      bool mean(const std::string &op, const std::string &chrom, const mongo::BSONObj &obj, const Region &region, std::string &result, std::string &msg);

      bool var(const std::string &op, const std::string &chrom, const mongo::BSONObj &obj, const Region &region, std::string &result, std::string &msg);

      bool sd(const std::string &op, const std::string &chrom, const mongo::BSONObj &obj, const Region &region, std::string &result, std::string &msg);

      bool count(const std::string &op, const std::string &chrom, const mongo::BSONObj &obj, const Region &region, std::string &result, std::string &msg);

    public:
      static bool is_meta(const std::string &s);
      bool process(const std::string &, const std::string &, const Region &region, std::string &result, std::string &msg);
      static bool build_column(const std::string &name, columns::ColumnTypePtr &column_type, std::string &msg);

      static bool build_column(const std::string &name, const std::string &default_value,
                        columns::ColumnTypePtr &column_type, std::string &msg);
    };
  }
}

#endif
