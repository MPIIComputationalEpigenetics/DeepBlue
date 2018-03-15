//
//  sequence_retriever.hpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 04.11.16.
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

#ifndef EPIDB_DBA_SEQUENCE_RETRIEVER_HPP
#define EPIDB_DBA_SEQUENCE_RETRIEVER_HPP

#include <unordered_map>
#include <string>
#include <vector>

#include <mongo/bson/bson.h>

#include "../datatypes/regions.hpp"

namespace epidb {
  namespace dba {
    namespace retrieve {

      class SequenceRetriever {
      private:
        std::unordered_map<std::string, std::tuple<size_t, size_t>> chunk_sizes_;
        std::unordered_map<std::string, mongo::OID> file_ids_;

        std::unordered_map<std::string, std::map<size_t, std::string> > chunks_;

        SequenceRetriever() {}

        ~SequenceRetriever() {}

        bool get_file_id(const std::string &filename, mongo::OID &oid, std::string &msg);

        bool get_chunk_size(const std::string &filename, size_t &chunk_size, size_t &length, std::string &msg);

        bool get_chunk(const std::string &filename, const size_t start, const size_t chunk_size,
                       std::string &chunk, std::string &msg);

        bool get_sequence(const std::string &filename,
                          const size_t start, const size_t end, std::string &sequence, std::string &msg);

      public:
        static SequenceRetriever& singleton();

        bool exists(const std::string &genome, const std::string &chromosome);

        bool retrieve(const std::string &genome, const std::string &chromosome,
                      const size_t start, const size_t end, std::string &sequence, std::string &msg);

        void invalidade_cache();

      };
    }
  }
}

#endif
