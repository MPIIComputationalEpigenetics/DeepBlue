//
//  retrieve.cpp
//  epidb
//
//  Created by Felipe Albrecht on 01.06.13.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include <algorithm>
#include <limits>
#include <numeric>
#include <string>
#include <vector>

#include <boost/ref.hpp>
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>

#include <mongo/bson/bson.h>
#include <mongo/client/dbclient.h>

#include "collections.hpp"
#include "config.hpp"
#include "helpers.hpp"
#include "key_mapper.hpp"
#include "queries.hpp"

#include "../extras/utils.hpp"

#include "../regions.hpp"
#include "../log.hpp"

#include "retrieve.hpp"

namespace epidb {
  namespace dba {
    namespace retrieve {
      const size_t BULK_SIZE = 20000;

      struct RegionProcess {
        Regions regions;

        RegionProcess()
        {
          regions = build_regions();
        }

        void read_region(const mongo::BSONObj &region_bson, const CollectionId &collection_id)
        {
          Region region(collection_id);

          for ( mongo::BSONObj::iterator i = region_bson.begin(); i.more(); ) {
            mongo::BSONElement e = i.next();

            // get fixed keyes
            if (e.fieldName() == KeyMapper::START()) {
              region.set_start(e.numberInt());
            } else if (e.fieldName() == KeyMapper::END()) {
              region.set_end(e.numberInt());
            } // get free data
            else if (e.type() == mongo::String) {
              region.set(e.fieldName(), e.str());
            } else if (e.type() == mongo::NumberDouble) {
              region.set(e.fieldName(), utils::double_to_string(e.numberDouble()));
            } else if (e.type() == mongo::NumberInt) {
              region.set(e.fieldName(), utils::integer_to_string(e.numberInt()));
            } else {
              region.set(e.fieldName(), e.toString(false));
            }
          }
          // region._data.shrink_to_fit();
          regions->push_back(region);
        }
      };

      bool get_regions_from_collection(const std::string &collection,
                                       const CollectionId collection_id, const std::string &chromosome,
                                       const mongo::BSONObj &regions_query,
                                       Regions &regions, std::vector<size_t> &offsets, std::string &msg)
      {
        mongo::ScopedDbConnection c(config::get_mongodb_server());

        RegionProcess rp;

        mongo::Query query = mongo::Query(regions_query).sort(KeyMapper::START());
        //mongo::Query query = mongo::Query(regions_query);
        int queryOptions = (int)( mongo::QueryOption_NoCursorTimeout | mongo::QueryOption_SlaveOk );

        std::auto_ptr<mongo::DBClientCursor> cursor( c->query(collection, query, 0, 0, NULL, queryOptions) );
        cursor->setBatchSize(BULK_SIZE);

        while ( cursor->more() ) {
          while (cursor->moreInCurrentBatch()) {
            mongo::BSONObj o = cursor->nextSafe();
            rp.read_region(o, collection_id);
          }
        }

        c.done();

        if (!rp.regions->empty()) {
          regions->insert(regions->end(), rp.regions->begin(), rp.regions->end());
          offsets.push_back(regions->size());
        }
        return true;
      }

      void get_regions_job(const std::string &genome,
                           const std::vector<std::string> &collections_id,
                           const boost::shared_ptr<std::vector<std::string> > chromosomes,
                           const mongo::BSONObj &regions_query,
                           boost::shared_ptr<ChromosomeRegionsList> &result)
      {
        for (std::vector<std::string>::const_iterator chrom_it = chromosomes->begin(); chrom_it != chromosomes->end(); chrom_it++) {
          /*
          size_t total_count(0);
          for (std::vector<std::string>::const_iterator id_it = collections_id.begin(); id_it != collections_id.end(); id_it++) {
            std::string collection = helpers::region_collection_name(genome, *id_it, *chrom_it);
            CollectionId collection_id = build_collection_id(*id_it);
            size_t count(0);
            count_regions(genome, *collection_id, *chrom_it, regions_query, count);
            total_count += count;
            std::cerr << "parial: " << count << " total: " << total_count << std::endl;
          }
          std::cerr << "total: " << total_count;
          */


          Regions regions = build_regions(); // <<<<
          std::vector<size_t> offsets;
          offsets.push_back(0);

          for (std::vector<std::string>::const_iterator id_it = collections_id.begin(); id_it != collections_id.end(); id_it++) {
            CollectionId collection_id = build_collection_id(*id_it);
            std::string collection = helpers::region_collection_name(genome, *id_it, *chrom_it);
            std::string msg;
            if (!get_regions_from_collection(collection, collection_id, *chrom_it, regions_query, regions, offsets, msg)) {
              EPIDB_LOG_ERR(msg);
              return;
            }
          }

          // ----
          while (offsets.size() > 2) {
            assert(offsets.back() == regions->size());
            assert(offsets.front() == 0);
            std::vector<size_t> new_offsets;
            size_t x = 0;
            while (x + 2 < offsets.size()) {
              std::inplace_merge(&regions->at(offsets.at(x))
                                 , &regions->at(offsets.at(x + 1))
                                 , &((*regions)[offsets.at(x + 2)]) // this *might* be at the end
                                );
              // now they are sorted, we just put offsets[x] and offsets[x+2] into the new offsets.
              // offsets[x+1] is not relevant any more
              new_offsets.push_back(offsets.at(x));
              new_offsets.push_back(offsets.at(x + 2));
              x += 2;
            }
            // if the number of offsets was odd, there might be a dangling offset
            // which we must remember to include in the new_offsets
            if (x + 2 == offsets.size()) {
              new_offsets.push_back(offsets.at(x + 1));
            }
            // assert(new_offsets.front() == 0);
            assert(new_offsets.back() == regions->size());
            offsets.swap(new_offsets);
          }

          // -----

          if (regions->size() > 0) {
            ChromosomeRegions chromossomeRegions(*chrom_it, regions);
            result->push_back(chromossomeRegions);
          }
        }
      }

      bool get_regions(const std::string &genome, std::vector<std::string> &collections_id,
                       std::vector<std::string> &chromosomes, const mongo::BSONObj &regions_query,
                       ChromosomeRegionsList &results, std::string &msg)
      {
        const size_t max_threads = 10;
        std::vector<boost::thread *> threads;
        std::vector<boost::shared_ptr<ChromosomeRegionsList> > result_parts;

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

          boost::shared_ptr<std::vector<std::string> > chrs(new std::vector<std::string>(start, end));
          boost::shared_ptr<ChromosomeRegionsList> result_part(new ChromosomeRegionsList);
          boost::thread *t = new boost::thread(&get_regions_job,
                                               boost::cref(genome), boost::cref(collections_id), chrs,
                                               boost::cref(regions_query), result_part);
          threads.push_back(t);
          result_parts.push_back(result_part);
        }

        // kill threads
        size_t thread_count = threads.size();
        for (size_t i = 0; i < thread_count; ++i) {
          threads[i]->join();
          delete threads[i];
        }

        // unite results
        std::vector<boost::shared_ptr<ChromosomeRegionsList> >::iterator it;
        for (it = result_parts.begin(); it != result_parts.end(); ++it) {
          ChromosomeRegionsList::iterator cit;
          for (cit = (**it).begin(); cit != (**it).end(); ++cit) {
            results.push_back(*cit);
          }
        }
        return true;
      }

      bool count_regions(const std::string &genome, const std::string &collection_id, const std::string &chromosome, const mongo::BSONObj &regions_query,
                         size_t &count)
      {
        mongo::ScopedDbConnection c(config::get_mongodb_server());
        std::string collection_name = helpers::region_collection_name(genome, collection_id, chromosome);
        std::cerr << collection_name << std::endl;
        count = c->count(collection_name, regions_query);
        c.done();
        return true;
      }

      void count_regions_job(const std::string &genome,
                             const std::vector<std::string> &collections_id,
                             const boost::shared_ptr<std::vector<std::string> > chromosomes,
                             const mongo::BSONObj &regions_query, boost::shared_ptr<std::vector<size_t> > results)
      {
        mongo::ScopedDbConnection c(config::get_mongodb_server());
        for (std::vector<std::string>::const_iterator c_it = collections_id.begin(); c_it != collections_id.end(); ++c_it) {
          size_t size = 0;
          for (std::vector<std::string>::const_iterator it = chromosomes->begin(); it != chromosomes->end(); ++it) {
            std::string collection_name = helpers::region_collection_name(genome, *c_it, *it);
            size_t partial_size = c->count(collection_name, regions_query);
            size += partial_size;
          }
          results->push_back(size);
        }
        c.done();
      }

      bool count_regions(const std::string &genome,
                         std::vector<std::string> &collections_id,
                         std::vector<std::string> &chromosomes, const mongo::BSONObj &regions_query,
                         size_t &size, std::string &msg)
      {
        const size_t max_threads = 10;
        std::vector<boost::thread *> threads;
        std::vector<boost::shared_ptr<std::vector<size_t> > > result_parts;

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
          boost::shared_ptr<std::vector<std::string> > chrs(new std::vector<std::string>(start, end));
          boost::shared_ptr<std::vector<size_t> > result_part(new std::vector<size_t>);
          boost::thread *t = new boost::thread(&count_regions_job,
                                               boost::cref(genome), boost::cref(collections_id), chrs,
                                               boost::cref(regions_query), result_part);
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
        std::vector<boost::shared_ptr<std::vector<size_t> > >::iterator it;
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

        mongo::ScopedDbConnection c(config::get_mongodb_server());
        std::auto_ptr<mongo::DBClientCursor> data_cursor = c->query(helpers::collection_name(Collections::SEQUENCES()) + ".files",
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

        mongo::ScopedDbConnection c(config::get_mongodb_server());

        mongo::BSONObj projection = BSON("chunkSize" << 1);
        std::auto_ptr<mongo::DBClientCursor> data_cursor = c->query(helpers::collection_name(Collections::SEQUENCES()) + ".files",
            mongo::Query(BSON("filename" << filename)), 0, 0, &projection);

        if (data_cursor->more()) {
          chunk_size = data_cursor->next().getField("chunkSize").Int();
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
        mongo::ScopedDbConnection c(config::get_mongodb_server());

        mongo::BSONObj projection = BSON("_id" << 1);
        std::auto_ptr<mongo::DBClientCursor> data_cursor = c->query(helpers::collection_name(Collections::SEQUENCES()) + ".files",
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

        mongo::ScopedDbConnection c(config::get_mongodb_server());

        mongo::BSONObj projection = BSON("data" << 1);
        std::auto_ptr<mongo::DBClientCursor> data_cursor = c->query(helpers::collection_name(Collections::SEQUENCES()) + ".chunks",
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
