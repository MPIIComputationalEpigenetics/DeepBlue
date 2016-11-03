//
//  retrieve.cpp
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

#include <algorithm>
#include <cstdlib>
#include <future>
#include <limits>
#include <numeric>
#include <memory>
#include <string>
#include <thread>
#include <tuple>
#include <vector>

#include <boost/ref.hpp>
#include <boost/thread.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>

#include <mongo/bson/bson.h>

#include "collections.hpp"
#include "config.hpp"
#include "helpers.hpp"
#include "key_mapper.hpp"
#include "queries.hpp"

#include "../errors.hpp"

#include "../connection/connection.hpp"

#include "../datatypes/regions.hpp"

#include "../extras/compress.hpp"
#include "../extras/utils.hpp"

#include "../parser/wig.hpp"

#include "../log.hpp"

#include "retrieve.hpp"

namespace epidb {
  namespace dba {
    namespace retrieve {

      void insert_bed_regions(const mongo::BSONObj& arrobj, Regions &_regions, size_t& _it_count, size_t& _it_size,
                              const Position _query_start, const Position _query_end, DatasetId dataset_id, const bool full_overlap);

      const size_t BULK_SIZE = 20000;

      struct RegionProcess {
        bool _full_overlap;
        size_t _it_count;
        size_t _it_size;
        Regions &_regions;
        Position _query_start;
        Position _query_end;

        RegionProcess(Regions &regions, Position query_start, Position query_end, bool full_overlap) :
          _full_overlap(full_overlap),
          _it_count(0),
          _it_size(0),
          _regions(regions),
          _query_start(query_start),
          _query_end(query_end)
        { }

        void read_region(const mongo::BSONObj &region_bson)
        {
          if (region_bson.hasField(KeyMapper::WIG_TRACK_TYPE())) {
            DatasetId dataset_id = region_bson[KeyMapper::DATASET()].Int();

            int track_type = region_bson[KeyMapper::WIG_TRACK_TYPE()].Int();

            // The following 4 BsonElements are optional, because it I use "numberInt()" rather Int()
            Position start = region_bson[KeyMapper::START()].numberInt();
            Length step = region_bson[KeyMapper::WIG_STEP()].numberInt();
            Length span = region_bson[KeyMapper::WIG_SPAN()].numberInt();
            Length size = region_bson[KeyMapper::FEATURES()].numberInt();

            int db_data_size;
            const char *data;
            lzo_bytep decompressed_data = NULL;
            bool compressed = false;
            if (region_bson.hasField(KeyMapper::WIG_COMPRESSED())) {
              compressed = true;
              const lzo_bytep compressed_data = (lzo_bytep) region_bson[KeyMapper::WIG_DATA()].binData(db_data_size);
              size_t real_size = region_bson[KeyMapper::WIG_DATA_SIZE()].Int();

              size_t uncompressed_size;
              decompressed_data = epidb::compress::decompress(compressed_data, db_data_size, real_size, uncompressed_size);
            } else {
              data = region_bson[KeyMapper::WIG_DATA()].binData(db_data_size);
            }

            const Position *starts;
            const Position *ends;
            const Score *scores;
            if (track_type  == parser::FIXED_STEP) {
              if (compressed) {
                scores = reinterpret_cast<const Score *>(decompressed_data);
              } else {
                scores = reinterpret_cast<const Score *>(data);
              }
              for (Length i = 0;
                   (i < size) &&
                   (start + (i * step) < _query_end);
                   i++) {

                if ((start + (i * step) < _query_end) &&  // Region START < Range END
                    (start + (i * step) + span > _query_start)) { // Region END > Range START
                  RegionPtr region = build_wig_region(start + (i * step), start + (i * step) + span, dataset_id, scores[i]);
                  _it_size += region->size();
                  _regions.emplace_back(std::move(region));
                  _it_count++;
                }
              }

            } else if (track_type == parser::VARIABLE_STEP) {
              if (compressed) {
                starts = reinterpret_cast<const Position *>(decompressed_data);
                scores = reinterpret_cast<const Score *>(decompressed_data + (size * sizeof(Position)));
              } else {
                starts = reinterpret_cast<const Position *>(data);
                scores = reinterpret_cast<const Score *>(data + (size * sizeof(Position)));
              }
              for (Length i = 0;
                   (i < size) &&
                   (starts[i] < _query_end);
                   i++) {

                if ((starts[i] < _query_end) &&  // Region START < Range END
                    ((starts[i] + span) > _query_start)) { // Region END > Range START

                  RegionPtr region = build_wig_region(starts[i], starts[i] + span, dataset_id, scores[i]);
                  _it_size += region->size();
                  _regions.emplace_back(std::move(region));
                  _it_count++;
                }
              }

            } else if ((track_type == parser::ENCODE_BEDGRAPH) || (track_type == parser::MISC_BEDGRAPH)) {
              if (compressed) {
                starts = reinterpret_cast<const Position *>(decompressed_data);
                ends = reinterpret_cast<const Position *>(decompressed_data + (size * sizeof(Position)));
                scores = reinterpret_cast<const Score *>(decompressed_data + (size * sizeof(Position) + (size * sizeof(Position))));
              } else {
                starts = reinterpret_cast<const Position *>(data);
                ends = reinterpret_cast<const Position *>(data + (size * sizeof(Position)));
                scores = reinterpret_cast<const Score *>(data + (size * sizeof(Position) + (size * sizeof(Position))));
              }

              for (Length i = 0;
                   (i < size) &&
                   (starts[i] < _query_end);
                   i++) {

                if ((starts[i] < _query_end) &&
                    (ends[i] > _query_start)) {

                  RegionPtr region = build_wig_region(starts[i], ends[i], dataset_id, scores[i]);
                  _it_size += region->size();
                  _regions.emplace_back(std::move(region));
                  _it_count++;
                }
              }
            }

            if (compressed) {
              free(decompressed_data);
            }
          }

          else if (region_bson.hasField(KeyMapper::BED_COMPRESSED())) {
            bool compressed = region_bson[KeyMapper::BED_COMPRESSED()].Bool();
            DatasetId dataset_id = region_bson[KeyMapper::DATASET()].Int();
            int db_data_size;

            if (compressed) {
              const lzo_bytep compressed_data = (lzo_bytep) region_bson[KeyMapper::BED_DATA()].binData(db_data_size);
              size_t real_size = region_bson[KeyMapper::BED_DATASIZE()].Int();

              size_t uncompressed_size;
              unsigned char *data = epidb::compress::decompress(compressed_data, db_data_size, real_size, uncompressed_size);
              mongo::BSONObj arrobj((char *) data);
              // TODO: check uncompressed_size == real_size

              insert_bed_regions(arrobj, _regions, _it_count, _it_size, _query_start, _query_end, dataset_id, _full_overlap);
              free(data);

              // Grouped in blocks but not compressed
            } else {
              const char *data = region_bson[KeyMapper::BED_DATA()].binData(db_data_size);

              mongo::BSONObj arrobj((char *) data);

              insert_bed_regions(arrobj, _regions, _it_count, _it_size, _query_start, _query_end, dataset_id, _full_overlap);
            }
          }
        }
      };

      inline void insert_bed_regions(const mongo::BSONObj& arrobj, Regions &_regions, size_t& _it_count, size_t& _it_size,
                                     const Position _query_start, const Position _query_end, DatasetId dataset_id, bool full_overlap)
      {
        mongo::BSONObj::iterator regions_it = arrobj.begin();
        while (regions_it.more()) {
          const mongo::BSONObj &region_bson = regions_it.next().Obj();

          mongo::BSONObj::iterator i = region_bson.begin();
          Position start = i.next().Int();
          Position end = i.next().Int();

          if (full_overlap) {
            if ((start < _query_start) || (end > _query_end)) {
              continue;
            }
          } else {
            if ((start >= _query_end) || (end <= _query_start)) {
              continue;
            }
          }

          RegionPtr region = build_bed_region(start, end, dataset_id);

          while ( i.more() ) {
            const mongo::BSONElement &e = i.next();
            switch (e.type()) {
            case mongo::String : region->insert(e.str()); break;
            case mongo::NumberDouble : region->insert((float) e._numberDouble()); break;
            case mongo::NumberInt : region->insert(e._numberInt()); break;
            default: region->insert(e.toString(false));
            }
          }
          _it_size += region->size();
          _regions.emplace_back(std::move(region));
          _it_count++;
        }
      }

      mongo::Query build_query(const mongo::BSONObj &regions_query, Position& start, Position& end)
      {
        if (regions_query.hasField(KeyMapper::START())) {
          mongo::BSONObj o = regions_query[KeyMapper::START()].Obj();
          end = o["$lte"].Int();
        } else {
          end = std::numeric_limits<Position>::max();
        }

        if (regions_query.hasField(KeyMapper::END())) {
          mongo::BSONObj o = regions_query[KeyMapper::END()].Obj();
          start = o["$gte"].Int();
        } else {
          start = std::numeric_limits<Position>::min();
        }

        return mongo::Query(regions_query)
               .sort(BSON(KeyMapper::DATASET() << 1 << KeyMapper::START() << 1 << KeyMapper::END() << 1))
               .hint("D_1_S_1_E_1");
      }

      bool get_regions_from_collection(const std::string &collection, const mongo::BSONObj &regions_query, const bool full_overlap,
                                       processing::StatusPtr status, Regions &regions, std::string &msg)
      {
        Position start;
        Position end;

        Connection c;
        unsigned long long count = c->count(collection, regions_query);
        if (count == 0) {
          c.done();
          return true;
        }

        mongo::Query query = build_query(regions_query, start, end);

        int queryOptions = (int)( mongo::QueryOption_NoCursorTimeout | mongo::QueryOption_SlaveOk );
        regions.reserve(count);
        auto cursor( c->query(collection, query, 0, 0, NULL, queryOptions) );
        cursor->setBatchSize(BULK_SIZE);
        RegionProcess rp(regions, start, end, full_overlap);
        while ( cursor->more() ) {
          while (cursor->moreInCurrentBatch()) {
            mongo::BSONObj o = cursor->nextSafe();
            rp.read_region(o);
            status->sum_regions(rp._it_count);

            // Check if processing was canceled
            bool is_canceled = false;
            if (!status->is_canceled(is_canceled, msg)) {
              c.done();
              return true;
            }
            if (is_canceled) {
              c.done();
              msg = Error::m(ERR_REQUEST_CANCELED);
              return false;
            }
            // ***

            // Check memory consumption
            if (status->sum_size(rp._it_size) < 0) {
              msg = "Memory exhausted. Used "  + utils::size_t_to_string(status->total_size()) + "bytes of " + utils::size_t_to_string(status->maximum_size()) + "bytes allowed. Please, select a smaller initial dataset, for example, selecting fewer chromosomes)"; // TODO: put a better error msg.
              c.done();
              return false;
            }
            // ***

            // Reset iteration stats
            rp._it_count = 0;
            rp._it_size = 0;
          }
        }

        std::sort(regions.begin(), regions.end(), RegionPtrComparer);

        c.done();
        return true;
      }

      std::tuple<bool, std::string> get_regions_job(const std::string &genome, const std::shared_ptr<std::vector<std::string> > chromosomes,
          const mongo::BSONObj &regions_query, const bool full_overlap,
          processing::StatusPtr status, std::shared_ptr<ChromosomeRegionsList> result)
      {

        for (std::vector<std::string>::const_iterator chrom_it = chromosomes->begin(); chrom_it != chromosomes->end(); chrom_it++) {
          std::string collection = helpers::region_collection_name(genome, *chrom_it);
          Regions regions = Regions();
          std::string msg;
          if (!get_regions_from_collection(collection, regions_query, full_overlap, status, regions, msg)) {
            return std::make_tuple(false, msg);
          }

          if (regions.size() > 0) {
            ChromosomeRegions chromosomeRegions(*chrom_it, std::move(regions));
            result->push_back(std::move(chromosomeRegions));
          }
        }

        return std::make_tuple(true, std::string(""));
      }

      bool get_regions_preview(const std::string &genome, const std::string &chromosome, const mongo::BSONObj &regions_query,
                               Regions& regions, std::string &msg)
      {

        std::string collection = helpers::region_collection_name(genome, chromosome);
        regions = Regions();

        Position start = 0;
        Position end = std::numeric_limits<Position>::max();

        mongo::Query query = build_query(regions_query, start, end);

        Connection c;
        mongo::BSONObj o = c->findOne(collection, query);
        c.done();

        RegionProcess rp(regions, start, end, true);
        rp.read_region(o);

        return true;
      }

      bool get_regions(const std::string &genome, const std::string &chromosome,
                       const mongo::BSONObj &regions_query, const bool full_overlap,
                       processing::StatusPtr status,
                       Regions &regions, std::string &msg)
      {
        std::string collection = helpers::region_collection_name(genome, chromosome);
        regions = Regions();
        if (!get_regions_from_collection(collection, regions_query, full_overlap, status, regions, msg)) {
          EPIDB_LOG_ERR(msg);
          return false;
        }
        std::cerr << "REGIONS AFTER: " << regions.size() << std::endl;
        return true;
      }

      bool get_regions(const std::string &genome, std::vector<std::string> &chromosomes,
                       const mongo::BSONObj &regions_query, const bool full_overlap,
                       processing::StatusPtr status, ChromosomeRegionsList &results, std::string &msg)
      {
        const size_t max_threads = 8;
        std::vector<std::future<std::tuple<bool, std::string> > > threads;
        std::vector<std::shared_ptr<ChromosomeRegionsList> > result_parts;

        size_t chunk_size = ceil(double(chromosomes.size()) / double(max_threads));

        for (size_t i = 0; i < max_threads; ++i) {
          std::vector<std::string>::iterator start = chromosomes.begin() + i * chunk_size;
          if (start >= chromosomes.end()) {
            break;
          }
          std::vector<std::string>::iterator end = start + chunk_size;
          if (end > chromosomes.end()) {
            end = chromosomes.end();
          }

          std::shared_ptr<std::vector<std::string> > chrs(new std::vector<std::string>(start, end));
          std::shared_ptr<ChromosomeRegionsList> result_part(new ChromosomeRegionsList);

          auto t = std::async(std::launch::async,
                              &get_regions_job,
                              std::ref(genome), chrs, std::ref(regions_query),
                              full_overlap, status, result_part);

          threads.push_back(std::move(t));
          result_parts.push_back(result_part);
        }

        // kill threads
        size_t thread_count = threads.size();
        for (size_t i = 0; i < thread_count; ++i) {
          threads[i].wait();
          auto result = threads[i].get();
          if (!std::get<0>(result)) {
            msg = std::get<1>(result);
            return false;
          }
        }

        // unite results
        for (auto &chromosome_regions_pre_result : result_parts) {
          for (auto &chromosome_regions_list : *chromosome_regions_pre_result) {
            results.push_back(std::move(chromosome_regions_list));
          }
        }
        return true;
      }

      bool count_regions(const std::string &genome, const std::string &chromosome, const mongo::BSONObj &regions_query, const bool full_overlap,
                         processing::StatusPtr status, size_t &count)
      {
        std::string collection_name = helpers::region_collection_name(genome, chromosome);
        std::string msg;
        Regions regions = Regions();
        if (!get_regions_from_collection(collection_name, regions_query, full_overlap, status, regions, msg)) {
          EPIDB_LOG_ERR(msg);
          count = 0;
          return false;
        }
        count = regions.size();
        return true;
      }

      void count_regions_job(const std::string &genome, const std::shared_ptr<std::vector<std::string> > chromosomes, const mongo::BSONObj &regions_query, const bool full_overlap,
                             processing::StatusPtr status, std::shared_ptr<std::vector<size_t> > results)
      {
        size_t size = 0;
        for (std::vector<std::string>::const_iterator it = chromosomes->begin(); it != chromosomes->end(); ++it) {
          std::string collection_name = helpers::region_collection_name(genome, *it);
          Regions regions = Regions();
          std::string msg;
          if (!get_regions_from_collection(collection_name, regions_query, full_overlap, status, regions, msg)) {
            EPIDB_LOG_ERR(msg);
            return;
          }
          size += regions.size();;
        }
        results->push_back(size);
      }

      bool count_regions(const std::string &genome, const std::vector<std::string> &chromosomes,
                         const mongo::BSONObj &regions_query, const bool full_overlap,
                         processing::StatusPtr status, size_t &size, std::string &msg)
      {
        const size_t max_threads = 8;
        std::vector<boost::thread *> threads;
        std::vector<std::shared_ptr<std::vector<size_t> > > result_parts;

        size_t chunk_size = ceil(double(chromosomes.size()) / double(max_threads));

        for (size_t i = 0; i < max_threads; ++i) {
          std::vector<std::string>::const_iterator start = chromosomes.begin() + i * chunk_size;
          if (start >= chromosomes.end()) {
            break;
          }
          std::vector<std::string>::const_iterator end = start + chunk_size;
          if (end > chromosomes.end()) {
            end = chromosomes.end();
          }
          std::shared_ptr<std::vector<std::string> > chrs(new std::vector<std::string>(start, end));
          std::shared_ptr<std::vector<size_t> > result_part(new std::vector<size_t>);
          boost::thread *t = new boost::thread(&count_regions_job, boost::cref(genome), chrs,
                                               boost::cref(regions_query), full_overlap, status, result_part);
          threads.push_back(t);
          result_parts.push_back(result_part);
        }

        // kill threads
        size_t thread_count = threads.size();
        for (size_t i = 0; i < thread_count; ++i) {
          threads[i]->join();
          delete threads[i];
        }

        size = 0;
        std::vector<std::shared_ptr<std::vector<size_t> > >::iterator it;
        for (it = result_parts.begin(); it != result_parts.end(); ++it) {
          std::vector<size_t> v = **it;
          size += std::accumulate(v.begin(), v.end(), 0ll);
        }
        return true;
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


      bool SequenceRetriever::get_chunk_size(const std::string &filename, size_t &chunk_size, std::string &msg)
      {
        if (chunk_sizes_.find(filename) != chunk_sizes_.end()) {
          chunk_size = chunk_sizes_.find(filename)->second;
          return true;
        }

        Connection c;

        mongo::BSONObj projection = BSON("chunkSize" << 1);
        auto data_cursor = c->query(helpers::collection_name(Collections::SEQUENCES()) + ".files",
                                    mongo::Query(BSON("filename" << filename)), 0, 0, &projection);

        if (data_cursor->more()) {
          chunk_size = data_cursor->next().getField("chunkSize").numberLong();
          chunk_sizes_[filename] = chunk_size;
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

      bool SequenceRetriever::get_chunk(const std::string &filename, size_t start,
                                        std::string &chunk, std::string &msg)
      {
        if (chunks_[filename].find(start) != chunks_[filename].end()) {
          chunk = chunks_[filename][start];
          return true;
        }

        size_t chunk_size;
        if (!get_chunk_size(filename, chunk_size, msg)) {
          return false;
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
        if (!get_chunk_size(filename, chunk_size, msg)) {
          return false;
        }

        sequence = "";

        size_t s = start;
        while (s <= end) {
          size_t chunk_start = s - (s % chunk_size);

          std::string chunk;
          if (!get_chunk(filename, s, chunk, msg)) {
            return false;
          }

          // will add the whole chunk or just a part if there is less
          // than a full chunk size left to get
          size_t first = (s == start) ? (s % chunk_size) : 0;
          size_t last = (end - chunk_start) > chunk_size ? chunk_size : first + end - s;
          sequence += chunk.substr(first, last - first);

          s += chunk_size;
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

    }
  }
}
