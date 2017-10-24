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

#include "../extras/utils.hpp"

#include <boost/serialization/bitset.hpp>

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

#define BITMAP_SIZE 1024 * 1024

namespace epidb {

  namespace signature {

    bool process_bitmap(const datatypes::User& user, const std::string &experiment_id,
                        std::bitset<BITMAP_SIZE>& out_bitmap,
                        processing::StatusPtr status, std::string& msg);

    bool list_similar_experiments(const datatypes::User& user, const std::string& query_id, const std::vector<utils::IdName>& names,
                                  std::vector<utils::IdNameCount> results, std::string& msg)
    {

      return true;
    }

    bool store(const datatypes::User& user, const std::string &experiment_id,
               processing::StatusPtr status, std::string& msg)
    {
      std::bitset<BITMAP_SIZE> bitmap;

      if (!process_bitmap(user, experiment_id, bitmap, status, msg)) {
        return false;
      }

      std::stringstream stream;
      boost::archive::binary_oarchive ar(stream, boost::archive::no_header);
      ar & bitmap;

      const char* data = stream.str().data();

      Connection c;
      auto doc = BSON("_id" << experiment_id << "data" << data);
      c->insert(dba::helpers::collection_name(dba::Collections::SIGNATURES()), doc);
      c.done();

      return true;
    }

    bool load(const std::string &experiment_id, std::bitset<BITMAP_SIZE>& bitset, std::string& msg)
    {
      auto query = BSON("_id" << experiment_id);
      mongo::BSONObj result;
      if (!dba::helpers::get_one(dba::helpers::collection_name(dba::Collections::SIGNATURES()), query, result)) {
        msg = Error::m(ERR_INVALID_EXPERIMENT_ID, experiment_id);
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
                      size_t pos, std::bitset<BITMAP_SIZE>& bitmap, std::string& string)
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

    bool process_bitmap_query(const datatypes::User& user, const std::string &query_id,
                              std::bitset<BITMAP_SIZE>& out_bitmap,
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

      size_t total_genome_size = 0;
      for (const auto &chromosome : chromosomes) {
        total_genome_size += chromosome.size;
      }

      size_t tiling_size = total_genome_size / BITMAP_SIZE;

      auto tiling_query = BSON("args" <<
                               BSON(
                                 "norm_genome" << norm_genome
                               ));

      ChromosomeRegionsList range_regions;
      if (!dba::query::retrieve_tiling_query(tiling_query, status, range_regions, msg)) {
        return false;
      }

      size_t pos = 0;
      std::bitset<BITMAP_SIZE> bitmap;

      ChromosomeRegionsList data_regions;
      if (!dba::query::retrieve_query(user, query_id, status, data_regions, msg, true)) {
        return false;
      }

      for (auto &chromosome : range_regions) {
        for (auto &datum: data_regions) {
          if (chromosome.first == datum.first) {
            if (!build_bitmap(chromosome.second, datum.second, pos, out_bitmap, msg)) {
              return false;
            }
          }
        }
      }

      out_bitmap = std::move(bitmap);
      return true;
    }

    bool process_bitmap_experiment(const datatypes::User& user, const std::string &id,
                                   std::bitset<BITMAP_SIZE>& out_bitmap,
                                   processing::StatusPtr status, std::string& msg)
    {
      mongo::BSONObj experiment_obj;
      if (!dba::experiments::by_id(id, experiment_obj, msg)) {
        return false;
      }

      const std::string& norm_exp_name = experiment_obj["norm_name"].String();
      const std::string& norm_genome = experiment_obj["norm_genome"].String();

      std::vector<dba::genomes::ChromosomeInfo> chromosomes;
      if (!dba::genomes::get_chromosomes(norm_genome, chromosomes, msg)) {
        return false;
      }

      size_t total_genome_size = 0;
      for (const auto &chromosome : chromosomes) {
        total_genome_size += chromosome.size;
      }

      size_t tiling_size = total_genome_size / BITMAP_SIZE;

      auto tiling_query = BSON("args" <<
                               BSON(
                                 "norm_genome" << norm_genome
                               ));

      ChromosomeRegionsList range_regions;
      if (!dba::query::retrieve_tiling_query(tiling_query, status, range_regions, msg)) {
        return false;
      }

      size_t pos = 0;
      std::bitset<BITMAP_SIZE> bitmap;
      for (auto &chromosome : range_regions) {
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
        }
      }

      out_bitmap = std::move(bitmap);
      return true;
    }
  };
};

