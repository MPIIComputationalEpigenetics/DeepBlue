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
#include <future>
#include <string>
#include <sstream>
#include <thread>

#include <mongo/bson/bson.h>
#include <mongo/client/dbclient.h>

#include "../connection/connection.hpp"

#include "../datatypes/user.hpp"

#include "../dba/collections.hpp"
#include "../dba/experiments.hpp"
#include "../dba/genomes.hpp"
#include "../dba/helpers.hpp"
#include "../dba/queries.hpp"

#include "../extras/math.hpp"
#include "../extras/utils.hpp"

#include "../threading/semaphore.hpp"

#include <boost/serialization/bitset.hpp>

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

#include "enrichment_result.hpp"

#include "../log.hpp"

#define BITMAP_SIZE (1024 * 1024)

namespace epidb {

  namespace processing {

    bool store(const std::string &id, const std::bitset<BITMAP_SIZE>& bitmap,  processing::StatusPtr status, std::string& msg);

    bool load(const std::string &id, std::bitset<BITMAP_SIZE>& bitset,  processing::StatusPtr status, std::string& msg);

    ProcessOverlapResult compare_to(const datatypes::User& user,
                                    const std::bitset<BITMAP_SIZE>& query_bitmap,
                                    const utils::IdName& exp,
                                    const ChromosomeRegionsList& bitmap_regions,
                                    processing::StatusPtr status, threading::SemaphorePtr sem,
                                    std::string& msg);

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



    bool enrich_regions_fast(const datatypes::User& user, const std::string& query_id, const std::vector<utils::IdName>& names,
                             processing::StatusPtr status,
                             mongo::BSONObj& result, std::string& msg)
    {
      processing::RunningOp runningOp = status->start_operation(processing::PROCESS_ENRICH_REGIONS_FAST);
      if (processing::is_canceled(status, msg)) {
        return false;
      }

      ChromosomeRegionsList bitmap_regions;
      if (!get_bitmap_regions(user, query_id, bitmap_regions, status, msg)) {
        return false;
      }

      std::bitset<BITMAP_SIZE> query_bitmap;
      if (!load(query_id, query_bitmap, status, msg)) {
        if (!process_bitmap_query(user, query_id, bitmap_regions, query_bitmap, status,  msg)) {
          return false;
        }
        if (!store(query_id, query_bitmap, status, msg)) {
          return false;
        }
      }

      std::vector<std::future<ProcessOverlapResult > > threads;


      runningOp.set_total_steps(names.size());

      threading::SemaphorePtr sem = threading::build_semaphore(16);
      for (const auto& exp: names) {
        utils::IdNameCount result;

        sem->down();
        auto t = std::async(std::launch::async, &compare_to,
                            std::ref(user),
                            std::ref(query_bitmap), std::ref(exp),
                            std::ref(bitmap_regions),
                            status, sem,
                            std::ref(msg));

        threads.emplace_back(std::move(t));
      }

      std::vector<std::tuple<std::string, size_t>> datasets_support;
      std::vector<std::tuple<std::string, double>> datasets_log_score;
      std::vector<std::tuple<std::string, double>> datasets_odds_score;

      std::vector<ProcessOverlapResult> results;

      for (size_t i = 0; i < threads.size(); ++i) {
        threads[i].wait();
        auto result = threads[i].get();
        runningOp.increment_step();
        std::string dataset = std::get<0>(result);

        // --------------------- [0] dataseset, [1] biosource, [2] epi mark, [3] description, [4] size, [5] database_name, [6] ///negative_natural_log, [5] log_odds_score, [6] a, [7] b, [8] c, [9] d)
        datasets_log_score.push_back(std::make_tuple(dataset, std::get<6>(result)));
        datasets_odds_score.push_back(std::make_tuple(dataset, std::get<7>(result)));
        datasets_support.push_back(std::make_tuple(dataset, std::get<8>(result)));

        results.emplace_back(std::move(result));
      }

      std::vector<std::shared_ptr<ExperimentResult>> experiment_results =
            sort_results(results, datasets_support, datasets_log_score, datasets_odds_score);

      mongo::BSONObjBuilder bob;

      mongo::BSONArrayBuilder ab;
      for (const auto& er: experiment_results) {
        ab.append(er->toBSON());
      }
      bob.append("results", ab.obj());

      result = bob.obj();

      return true;
    }

    ProcessOverlapResult compare_to(const datatypes::User& user,
                                    const std::bitset<BITMAP_SIZE>& query_bitmap,
                                    const utils::IdName& exp,
                                    const ChromosomeRegionsList& bitmap_regions,
                                    processing::StatusPtr status, threading::SemaphorePtr sem,
                                    std::string& msg)
    {
      processing::RunningOp runningOp = status->start_operation(processing::PROCESS_ENRICH_REGIONS_FAST_COMPARE_TO);
      if (processing::is_canceled(status, msg)) {
        return std::make_tuple(exp.name, "", "", "", -1.0, "", -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, true, msg);
      }

      std::bitset<BITMAP_SIZE> exp_bitmap;

      if (!load(exp.id, exp_bitmap, status, msg)) {
        if (!process_bitmap_experiment(user, exp.id, bitmap_regions, exp_bitmap, status, msg)) {
          sem->up();
          return std::make_tuple(exp.name, "", "", "", -1.0, "", -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, true, msg);
        }
        if (!store(exp.id, exp_bitmap, status, msg)) {
          sem->up();
          return std::make_tuple(exp.name, "", "", "", -1.0, "", -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, true, msg);
        }
      }

      mongo::BSONObj experiment_obj;
      if (!dba::experiments::by_name(exp.name, experiment_obj, msg)) {
        sem->up();
        return std::make_tuple(exp.name, "", "", "", -1.0, "", -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, true, msg);
      }

      std::string biosource;
      std::string epigenetic_mark;
      std::string description;
      if (!experiment_obj.isEmpty()) {
        biosource = experiment_obj["sample_info"]["biosource_name"].String();
        epigenetic_mark = experiment_obj["epigenetic_mark"].String();
        description = experiment_obj["description"].String();
      }

      size_t count = (query_bitmap & exp_bitmap).count();

      double a = count;
      double b = exp_bitmap.count() - a;

      if (b < 0) {
        sem->up();
        msg = "Negative b entry in table. This means either: 1) Your user sets contain items outside your universe; or 2) your universe has a region that overlaps multiple user set regions, interfering with the universe set overlap calculation.";
        return std::make_tuple(exp.name, biosource, epigenetic_mark, description, -1, "", -1, -1, -1, -1, -1, -1, true, msg);
      }

      double c = query_bitmap.count()  - a;
      double d = BITMAP_SIZE - a - b - c;

      double p_value = math::fisher_test(a, b, c, d);

      double negative_natural_log = abs(log10(p_value));

      double a_b = a/b;
      double c_d = c/d;
      double log_odds_score;
      // Handle when 'b' and 'd' are 0.
      if (a_b == c_d) {
        log_odds_score = 1;
      } else {
        log_odds_score = a_b/c_d;
      }

      sem->up();
      return std::make_tuple(exp.name, biosource, epigenetic_mark, "", BITMAP_SIZE, "", negative_natural_log, log_odds_score, a, b, c, d, false, msg);
    }

    bool store(const std::string &id, const std::bitset<BITMAP_SIZE>& bitmap, processing::StatusPtr status, std::string& msg)
    {
      processing::RunningOp runningOp = status->start_operation(processing::PROCESS_ENRICH_REGIONS_FAST_STORE_BITMAP);
      if (processing::is_canceled(status, msg)) {
        return false;
      }

      std::stringstream stream;
      boost::archive::binary_oarchive ar(stream, boost::archive::no_header);
      ar & bitmap;

      size_t size = stream.str().size();
      const char* data = stream.str().data();

      mongo::BSONObjBuilder bob;
      bob.append("_id", id);
      bob.appendBinData("data", size, mongo::BinDataGeneral, (void *) data);

      Connection c;
      try {
        c->insert(dba::helpers::collection_name(dba::Collections::SIGNATURES()), bob.obj());
      } catch (const mongo::OperationException& e ) {
        const auto& info = e.obj();
        if (info["code"].Int() == 11000) {
          EPIDB_LOG_TRACE("Error while inserting the bitmap for the ID:" + id + ". It was already inserted.");
        } else {
          throw e;
        }
      }
      c.done();

      return true;
    }

    bool load(const std::string &id, std::bitset<BITMAP_SIZE>& bitset, processing::StatusPtr status, std::string& msg)
    {
      processing::RunningOp runningOp = status->start_operation(processing::PROCESS_ENRICH_REGIONS_FAST_LOAD_BITMAP);
      if (processing::is_canceled(status, msg)) {
        return false;
      }

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
                      size_t &pos, std::bitset<BITMAP_SIZE>& bitmap,
                      processing::StatusPtr status, std::string& msg)
    {
      processing::RunningOp runningOp = status->start_operation(processing::PROCESS_ENRICH_REGIONS_FAST_BUILD_BITMAP);
      if (processing::is_canceled(status, msg)) {
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
          if (pos >= BITMAP_SIZE) {
            msg = "Invalid position - " + utils::integer_to_string(pos);
            return false;
          }
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
      processing::RunningOp runningOp = status->start_operation(processing::PROCESS_ENRICH_REGIONS_FAST_GET_BITMAP_REGIONS);
      if (processing::is_canceled(status, msg)) {
        return false;
      }

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

      size_t d = total_genome_size / BITMAP_SIZE;
      size_t tiling_size = d + chromosomes.size() + 1;

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
      processing::RunningOp runningOp = status->start_operation(processing::PROCESS_ENRICH_REGIONS_FAST_BITMAP_QUERY);
      if (processing::is_canceled(status, msg)) {
        return false;
      }

      size_t pos = 0;
      ChromosomeRegionsList data_regions;
      if (!dba::query::retrieve_query(user, query_id, status, data_regions, msg, true)) {
        return false;
      }

      for (auto &chromosome : bitmap_regions) {
        bool found = false;
        for (auto &datum: data_regions) {
          if (chromosome.first == datum.first) {
            if (!build_bitmap(chromosome.second, datum.second, pos, out_bitmap, status, msg)) {
              msg += " (Query ID: " + query_id + ", chromosome: " + chromosome.first + ")";
              return false;
            }
            for (const auto& r : datum.second) {
              status->subtract_size(r->size());
            }
            found = true;
          }
        }
        if (!found) {
          Regions empty_data;
          if (!build_bitmap(chromosome.second, empty_data, pos, out_bitmap, status, msg)) {
            msg += " (Query ID: " + query_id + ", chromosome: " + chromosome.first + "(empty))";
            return false;
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
      processing::RunningOp runningOp = status->start_operation(processing::PROCESS_ENRICH_REGIONS_FAST_BITMAP_EXPERIMENT);
      if (processing::is_canceled(status, msg)) {
        return false;
      }

      mongo::BSONObj experiment_obj;
      if (!dba::experiments::by_id(id, experiment_obj, msg)) {
        return false;
      }
      const std::string& norm_exp_name = experiment_obj["norm_name"].String();
      const std::string& norm_genome = experiment_obj["norm_genome"].String();

      size_t pos = 0;
      for (auto &chromosome : bitmap_regions) {
        mongo::BSONObj regions_query;
        if (!dba::query::build_experiment_query(-1, -1, norm_exp_name, regions_query, msg)) {
          return false;
        }

        Regions data;
        if (!dba::retrieve::get_regions(norm_genome, chromosome.first, regions_query, false, status, data, msg, true)) {
          return false;
        }

        if (!build_bitmap(chromosome.second, data,  pos, out_bitmap, status, msg)) {
          msg += " (Query ID: " + id + ", chromosome: " + chromosome.first + "(empty))";
          return false;
        }

        for (const auto& r : data) {
          status->subtract_size(r->size());
        }
      }

      return true;
    }
  };
};
