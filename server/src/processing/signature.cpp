//
//  signature.hpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 21.10.17.
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


#include <bitset>
#include <string>
#include <sstream>

#include <mongo/bson/bson.h>

#include "../connection/connection.hpp"

#include "../datatypes/user.hpp"

#include "../dba/collections.hpp"
#include "../dba/experiments.hpp"
#include "../dba/genomes.hpp"
#include "../dba/helpers.hpp"
#include "../dba/queries.hpp"

#include "../extras/math.hpp"
#include "../extras/utils.hpp"

#include <boost/serialization/bitset.hpp>

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

#define BITMAP_SIZE (1024 * 1024)

namespace epidb {

  namespace processing {

    bool store(const std::string &id, const std::bitset<BITMAP_SIZE>& bitmap, std::string& msg);

    bool load(const std::string &id, std::bitset<BITMAP_SIZE>& bitset, std::string& msg);

    bool compare_to(const datatypes::User& user,
                    const std::bitset<BITMAP_SIZE>& query_bitmap,
                    const utils::IdName& exp,
                    const ChromosomeRegionsList& bitmap_regions,
                    processing::StatusPtr status,
                    utils::IdNameCount& result, std::string& msg);

    bool get_bitmap_regions(const datatypes::User& user, const std::string &query_id,
                            ChromosomeRegionsList& bitmap_regions,
                            processing::StatusPtr status, std::string& msg);

    bool process_bitmap_query(const datatypes::User& user, const std::string &query_id,
                              const ChromosomeRegionsList& bitmap_regions,
                              std::bitset<BITMAP_SIZE>& out_bitmap,
                              processing::StatusPtr status, std::string& msg);

    bool process_bitmap_experiment(const datatypes::User& user, const std::string &id,
                                   const ChromosomeRegionsList& bitmap_regions,
                                   std::bitset<BITMAP_SIZE>& out_bitmap,
                                   processing::StatusPtr status, std::string& msg);




    bool list_similar_experiments(const datatypes::User& user, const std::string& query_id, const std::vector<utils::IdName>& names,
                                  processing::StatusPtr status,
                                  std::vector<utils::IdNameCount> results, std::string& msg)
    {
      ChromosomeRegionsList bitmap_regions;
      if (!get_bitmap_regions(user, query_id, bitmap_regions, status, msg)) {
        return false;
      }

      std::bitset<BITMAP_SIZE> query_bitmap;
      if (!load(query_id, query_bitmap, msg)) {
        if (!process_bitmap_query(user, query_id, bitmap_regions, query_bitmap, status,  msg)) {
          return false;
        }
        if (!store(query_id, query_bitmap, msg)) {
          return false;
        }
      }

      std::cerr << "query count " << query_bitmap.count() << std::endl;

      for (const auto& exp: names) {
        utils::IdNameCount result;
        if (!compare_to(user, query_bitmap, exp, bitmap_regions, status, result, msg)) {
          return false;
        }

        std::cerr << result.name << " exp count: " << result.count << std::endl;
      }

      return true;
    }

    ProcessOverlapResult compare_to(const datatypes::User& user,
                    const std::bitset<BITMAP_SIZE>& query_bitmap,
                    const utils::IdName& exp,
                    const ChromosomeRegionsList& bitmap_regions,
                    processing::StatusPtr status, threading::SemaphorePtr sem,
                    std::string& msg)
    {
      std::bitset<BITMAP_SIZE> exp_bitmap;

      if (!load(exp.id, exp_bitmap, msg)) {
        if (!process_bitmap_experiment(user, exp.id, bitmap_regions, exp_bitmap, status, msg)) {
          sem->up();
          return std::make_tuple(msg, "", "", "", -1.0, "", -1.0, -1.0, -1.0, -1.0, -1.0, -1.0);
        }
        if (!store(exp.id, exp_bitmap, msg)) {
          sem->up();
          return std::make_tuple(msg, "", "", "", -1.0, "", -1.0, -1.0, -1.0, -1.0, -1.0, -1.0);
        }
      }

      size_t count = (query_bitmap & exp_bitmap).count();

      double a = count;
      double b = exp_bitmap.count() - a;

      if (b < 0) {
        sem->up();
        msg = "Negative b entry in table. This means either: 1) Your user sets contain items outside your universe; or 2) your universe has a region that overlaps multiple user set regions, interfering with the universe set overlap calculation.";
        return std::make_tuple(msg, "", "", "", -1.0, "", -1.0, -1.0, -1.0, -1.0, -1.0, -1.0);
      }

      double c = query_bitmap.count()  - a;
      double d = BITMAP_SIZE - a - b - c;

      double p_value = math::fisher_test(a, b, c, d);

      double negative_natural_log = abs(log10(p_value));

      double log_odds_score = (a/b)/(c/d);

      std::cerr << a << "\t" << b<< "\t" << c<< "\t" << d<< std::endl;

      sem->up();
      return std::make_tuple(dataset_name, biosource, epigenetic_mark, description, count_database_regions, database_name, negative_natural_log, log_odds_score, a, b, c, d);
    }

    bool store(const std::string &id, const std::bitset<BITMAP_SIZE>& bitmap, std::string& msg)
    {
      std::stringstream stream;
      boost::archive::binary_oarchive ar(stream, boost::archive::no_header);
      ar & bitmap;

      size_t size = stream.str().size();
      const char* data = stream.str().data();

      mongo::BSONObjBuilder bob;
      bob.append("_id", id);
      bob.appendBinData("data", size, mongo::BinDataGeneral, (void *) data);

      Connection c;
      c->insert(dba::helpers::collection_name(dba::Collections::SIGNATURES()), bob.obj());
      c.done();
      return true;
    }

    bool load(const std::string &id, std::bitset<BITMAP_SIZE>& bitset, std::string& msg)
    {
      auto query = BSON("_id" << id);
      mongo::BSONObj result;
      if (!dba::helpers::get_one(dba::Collections::SIGNATURES(), query, result)) {
        msg = Error::m(ERR_INVALID_EXPERIMENT_ID, id);
        return false;
      }

      int size;
      const auto data = result["data"].binData(size);
      std::istringstream ss(std::string(data,size));

      boost::archive::binary_iarchive ar(ss, boost::archive::no_header);
      ar & bitset;

      return true;
    }


    bool build_bitmap(const Regions& ranges, const Regions& data,
                      size_t &pos, std::bitset<BITMAP_SIZE>& bitmap, std::string& string)
    {
      auto it_ranges = ranges.begin();
      auto it_data_begin = data.begin();

      while (it_ranges != ranges.end()) {
        bool overlap = false;
        while ( it_data_begin != data.end()
                && (*it_data_begin)->end() < (*it_ranges)->start() )  {
          it_data_begin++;
        }

        auto it_data = it_data_begin;
        while (it_data != data.end() &&
               (*it_ranges)->end() >= (*it_data)->start() ) {

          if (((*it_ranges)->start() <= (*it_data)->end()) && ((*it_ranges)->end() >= (*it_data)->start())) {
            overlap = true;
          }
          it_data++;
        }
        if (overlap) {
          bitmap.set(pos);
        }

        pos++;
        it_ranges++;
      }

      return true;
    }

    bool get_bitmap_regions(const datatypes::User& user, const std::string &query_id,
                            ChromosomeRegionsList& bitmap_regions,
                            processing::StatusPtr status, std::string& msg)
    {
      std::string norm_genome;

      std::vector<std::string> vector_genome;
      const std::string em_field_name = "norm_genome";
      if (!dba::query::get_main_experiment_data(user, query_id, em_field_name, status, vector_genome, msg)) {
        return false;
      }

      if (vector_genome.empty()) {
        msg = "It is not possible to obtain the genome of the query " + query_id;
        return false;
      }

      norm_genome = vector_genome[0];

      std::vector<dba::genomes::ChromosomeInfo> chromosomes;
      if (!dba::genomes::get_chromosomes(norm_genome, chromosomes, msg)) {
        return false;
      }

      double total_genome_size = 0;
      for (const auto &chromosome : chromosomes) {
        total_genome_size += chromosome.size;
      }

      double tiling_size = total_genome_size / BITMAP_SIZE;

      auto tiling_query = BSON("args" <<
                               BSON(
                                 "norm_genome" << norm_genome <<
                                 "size" << (int) tiling_size
                               ));
      if (!dba::query::retrieve_tiling_query(tiling_query, status, bitmap_regions, msg)) {
        return false;
      }

      return true;
    }

    bool process_bitmap_query(const datatypes::User& user, const std::string &query_id,
                              const ChromosomeRegionsList& bitmap_regions,
                              std::bitset<BITMAP_SIZE>& out_bitmap,
                              processing::StatusPtr status, std::string& msg)
    {
      size_t pos = 0;
      ChromosomeRegionsList data_regions;
      if (!dba::query::retrieve_query(user, query_id, status, data_regions, msg, true)) {
        return false;
      }

      for (auto &chromosome : bitmap_regions) {
        for (auto &datum: data_regions) {
          if (chromosome.first == datum.first) {
            if (!build_bitmap(chromosome.second, datum.second, pos, out_bitmap, msg)) {
              return false;
            }
          }
        }
      }

      return true;
    }

    bool process_bitmap_experiment(const datatypes::User& user, const std::string &id,
                                   const ChromosomeRegionsList& bitmap_regions,
                                   std::bitset<BITMAP_SIZE>& out_bitmap,
                                   processing::StatusPtr status, std::string& msg)
    {
      mongo::BSONObj experiment_obj;
      if (!dba::experiments::by_id(id, experiment_obj, msg)) {
        return false;
      }
      const std::string& norm_exp_name = experiment_obj["norm_name"].String();
      const std::string& norm_genome = experiment_obj["norm_genome"].String();

      size_t pos = 0;
      for (auto &chromosome : bitmap_regions) {
        const int BLOCK_SIZE = 100;

        for (size_t region_pos = 0; region_pos < chromosome.second.size(); region_pos++) {
          Regions ranges;
          for (size_t i = 0; i < BLOCK_SIZE && region_pos < chromosome.second.size(); i++, region_pos++) {
            ranges.emplace_back(chromosome.second[region_pos]->clone());
          }

          Regions data;

          mongo::BSONObj regions_query;
          if (!dba::query::build_experiment_query(ranges[0]->start(), ranges[ranges.size() - 1]->end(),
                                                  norm_exp_name, regions_query, msg)) {
            return false;
          }
          if (!dba::retrieve::get_regions(norm_genome, chromosome.first, regions_query, false, status, data, msg)) {
            return false;
          }

          if (!build_bitmap(ranges, data,  pos, out_bitmap, msg)) {
            return false;
          }

          // Remove the size of the region, keeping only the size of the Stored score.
          for (const auto& r : data) {
            status->subtract_size(r->size() - sizeof(Score));
          }
        }

      }

      return true;
    }
  };
};

