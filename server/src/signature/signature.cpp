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
#include <mongo/bson/bson.h>

#include "../datatypes/user.hpp"

#include "../dba/experiments.hpp"
#include "../dba/genomes.hpp"
#include "../dba/queries.hpp"

#include <boost/serialization/bitset.hpp>

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

#define BITMAP_SIZE 1024 * 1024

namespace epidb {

  namespace signature {

    void serialize(boost::archive::binary_oarchive& ar, const std::bitset<BITMAP_SIZE>& bitset, const unsigned version)
    {
      ar & bitset;
    }

    void load(const datatypes::User& user, const std::string &experiment_id,
              boost::archive::binary_iarchive& ar, std::bitset<BITMAP_SIZE>& bitset, const unsigned version)
    {
      //std::istringstream ss(std::string(buf,len));
      ar & bitset;
    }


    bool process_bitmap(const datatypes::User& user,
                        const std::string &experiment_id, std::bitset<BITMAP_SIZE>& out_bitmap,
                        processing::StatusPtr status, std::string& msg)
    {
      mongo::BSONObj experiment_obj;
      if (!dba::experiments::by_id(experiment_id, experiment_obj, msg)) {
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

          mongo::BSONObj regions_query;
          if (!dba::query::build_experiment_query(ranges[0]->start(), ranges[ranges.size() - 1]->end(),
                                                  norm_exp_name, regions_query, msg)) {
            return false;
          }

          Regions data;
          if (!dba::retrieve::get_regions(norm_genome, chromosome.first, regions_query, false, status, data, msg)) {
            return false;
          }

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
        }
      }

      out_bitmap = std::move(bitmap);
      return true;
    }
  };
};

