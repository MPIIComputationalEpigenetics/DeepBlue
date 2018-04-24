//
//  sequence_retriever.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 01.06.13.
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

#include <limits>
#include <string>

#include <mongo/bson/bson.h>

#include "collections.hpp"
#include "helpers.hpp"
#include "key_mapper.hpp"

#include "../errors.hpp"

#include "../config/config.hpp"

#include "../connection/connection.hpp"

#include "../log.hpp"

#include "sequence_retriever.hpp"

namespace epidb {
  namespace dba {
    namespace retrieve {

      SequenceRetriever& SequenceRetriever::singleton()
      {
        static SequenceRetriever DEFAULT_RETRIEVER;
        return DEFAULT_RETRIEVER;
      }

      bool SequenceRetriever::exists(const std::string &genome, const std::string &chromosome)
      {
        std::string norm_genome = utils::normalize_name(genome);
        std::string filename = norm_genome + "." + chromosome;

        Connection c;
        auto data_cursor = c->query(helpers::collection_name(Collections::SEQUENCES()) + ".files",
                                    mongo::Query(BSON("filename" << filename)));

        bool r = data_cursor->more();

        c.done();
        return r;
      }


      bool SequenceRetriever::get_chunk_size(const std::string &filename, size_t &chunk_size, size_t &length, std::string &msg)
      {
        if (chunk_sizes_.find(filename) != chunk_sizes_.end()) {
          auto t = chunk_sizes_.find(filename)->second;
          chunk_size = std::get<0>(t);
          length = std::get<1>(t);
          return true;
        }

        Connection c;

        mongo::BSONObj projection = BSON("chunkSize" << 1 << "length" << 1);
        auto data_cursor = c->query(helpers::collection_name(Collections::SEQUENCES()) + ".files",
                                    mongo::Query(BSON("filename" << filename)), 0, 0, &projection);

        if (data_cursor->more()) {
          auto data = data_cursor->next();
          chunk_size = data.getField("chunkSize").numberLong();
          length = data.getField("length").numberLong();
          chunk_sizes_[filename] = std::make_tuple(chunk_size, length);
          c.done();
          return true;
        }
        msg = "Sequence file with name " + filename + " was not found";
        c.done();
        return false;
      }

      bool SequenceRetriever::get_file_id(const std::string &filename, mongo::OID &oid, std::string &msg)
      {
        if (file_ids_.find(filename) != file_ids_.end()) {
          oid = file_ids_.find(filename)->second;
          return true;
        }
        Connection c;

        mongo::BSONObj projection = BSON("_id" << 1);
        auto data_cursor = c->query(helpers::collection_name(Collections::SEQUENCES()) + ".files",
                                    mongo::Query(BSON("filename" << filename)), 0, 0, &projection);

        if (data_cursor->more()) {
          oid = data_cursor->next().getField("_id").OID();
          file_ids_[filename] = oid;
          c.done();
          return true;
        }
        msg = "Sequence file with name " + filename + " was not found";
        c.done();
        return false;
      }

      bool SequenceRetriever::get_chunk(const std::string &filename, const size_t start, const size_t chunk_size,
                                        std::string &chunk, std::string &msg)
      {
        if (chunks_[filename].find(start) != chunks_[filename].end()) {
          chunk = chunks_[filename][start];
          return true;
        }

        if (chunk_size > (size_t) std::numeric_limits<int>::max()) {
          msg = "Internal Error. Chuck size is too big. Please, ask the administrator to check the logs.";
          EPIDB_LOG_ERR("Chuck Size too big: " << chunk_size);
          return false;
        }

        mongo::OID oid;
        if (!get_file_id(filename, oid, msg)) {
          return false;
        }
        size_t chunk_number = start / chunk_size;

        Connection c;

        mongo::BSONObj projection = BSON("data" << 1);
        auto data_cursor = c->query(helpers::collection_name(Collections::SEQUENCES()) + ".chunks",
                                    mongo::Query(BSON("files_id" << oid << "n" << (int) chunk_number)), 0, 0, &projection);

        if (data_cursor->more()) {
          int i_chuck_size = (int) chunk_size;
          chunk = data_cursor->next().getField("data").binData(i_chuck_size);
          chunks_[filename][start] = chunk;
          c.done();
          return true;
        }
        c.done();

        msg = "Chunk for file " + filename + " not found.";
        return false;
      }

      bool SequenceRetriever::get_sequence(const std::string &filename,
                                           const size_t start, const size_t end, std::string &sequence, std::string &msg)
      {
        size_t chunk_size;
        size_t length;
        if (!get_chunk_size(filename, chunk_size, length, msg)) {
          return false;
        }

        sequence = "";

        size_t s = start;
        size_t real_end = std::min(end, length);
        while (s < real_end) {
          size_t chunk_start = s - (s % chunk_size);

          std::string chunk;
          if (!get_chunk(filename, s, chunk_size, chunk, msg)) {
            return false;
          }

          // will add the whole chunk or just a part if there is less
          // than a full chunk size left to get
          size_t first = (s == start) ? (s % chunk_size) : 0;
          size_t last = (real_end - chunk_start) > chunk_size ? chunk_size : first + real_end - s;
          std::string chuck_substr = chunk.substr(first, last - first);
          sequence += chuck_substr;
          s += chuck_substr.size();
        }
        return true;
      }

      bool SequenceRetriever::retrieve(const std::string &genome, const std::string &chromosome,
                                       const size_t start, const size_t end, std::string &sequence, std::string &msg)
      {
        std::string norm_genome = utils::normalize_name(genome);
        std::string filename = norm_genome + "." + chromosome;

        if (!get_sequence(filename, start, end, sequence, msg)) {
          return false;
        }
        return true;
      }

      void SequenceRetriever::invalidade_cache()
      {
        chunk_sizes_.clear();
        file_ids_.clear();
        chunks_.clear();
      }

    }
  }
}
