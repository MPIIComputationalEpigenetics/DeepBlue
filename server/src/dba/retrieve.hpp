//
//  retrieve.hpp
//  epidb
//
//  Created by Felipe Albrecht on 01.06.13.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#ifndef EPIDB_DBA_RETRIEVE_HPP
#define EPIDB_DBA_RETRIEVE_HPP

#include <map>
#include <string>
#include <vector>

#include <mongo/bson/bson.h>

#include "../datatypes/regions.hpp"
#include "../processing/processing.hpp"


namespace epidb {
  namespace dba {
    namespace retrieve {

      bool get_regions(const std::string &genome, std::string &chromosome,
                       const mongo::BSONObj &regions_query, const bool full_overlap,
                       processing::StatusPtr status,
                       Regions &regions, std::string &msg);

      bool get_regions(const std::string &genome, std::vector<std::string> &chromosomes,
                       const mongo::BSONObj &regions_query, const bool full_overlap,
                       processing::StatusPtr status,
                       ChromosomeRegionsList &results, std::string &msg);

      bool count_regions(const std::string &genome, const std::string &chromosome,
                         const mongo::BSONObj &regions_query, const bool full_overlap,
                         processing::StatusPtr status,
                         size_t &count);

      bool count_regions(const std::string &genome,
                         std::vector<std::string> &chromosomes,
                         const mongo::BSONObj &regions_query, const bool full_overlap,
                         processing::StatusPtr status,
                         size_t &size, std::string &msg);


      class SequenceRetriever {
      private:
        std::map<std::string, size_t> chunk_sizes_;
        std::map<std::string, mongo::OID> file_ids_;

        std::map<std::string, std::map<size_t, std::string> > chunks_;

        bool get_file_id(const std::string &filename, mongo::OID &oid, std::string &msg);

        bool get_chunk_size(const std::string &filename, size_t &chunk_size, std::string &msg);

        bool get_chunk(const std::string &filename, size_t start,
                       std::string &chunk, std::string &msg);

        bool get_sequence(const std::string &filename,
                          const size_t start, const size_t end, std::string &sequence, std::string &msg);

      public:
        SequenceRetriever() {}

        ~SequenceRetriever() {}

        bool exists(const std::string &genome, const std::string &chromosome);

        bool retrieve(const std::string &genome, const std::string &chromosome,
                      const size_t start, const size_t end, std::string &sequence, std::string &msg);

      };
    }
  }
}

#endif
