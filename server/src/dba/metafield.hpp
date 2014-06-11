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

#include "../dba/retrieve.hpp"

#include "../regions.hpp"

namespace epidb {
  namespace dba {
    class Metafield {

    private:
      typedef bool (Metafield::*Function)(const std::string &, const std::string &, const mongo::BSONObj &, const Region &, std::string &, std::string &);

      static std::map<const std::string, Function> functions;

      std::map<CollectionId, mongo::BSONObj> obj_by_collection_id;

      dba::retrieve::SequenceRetriever seq_retr;

      static const std::map<const std::string, Function> createFunctionsMap();

      bool get_bson_by_collection_id(CollectionId collection_id, mongo::BSONObj &obj, std::string &msg);

      bool count_pattern(const std::string& pattern, const std::string& genome, const std::string &chrom,
                                       const Region &region, const bool overlap,
                                       size_t& count, std::string& msg);

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
    };
  }
}

#endif
