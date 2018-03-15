//
//  running_cache.hpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 01.11.16.
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

#include <memory>
#include <boost/regex.hpp>
#include <string>

#include "../dba/genomes.hpp"
#include "../dba/retrieve.hpp"
#include "../dba/sequence_retriever.hpp"
#include "../dba/key_mapper.hpp"

#include "processing.hpp"

#include "running_cache.hpp"

namespace epidb {
  namespace processing {

    bool DatasetCache::load_sequence(const std::string& chromosome, std::string& msg)
    {
      size_t size;
      if (!dba::genomes::chromosome_size(_genome, chromosome, size, msg)) {
        return false;
      }

      return dba::retrieve::SequenceRetriever::singleton().retrieve(_genome, chromosome, 0, size, _sequence, msg);
    }

    bool DatasetCache::count_regions(const std::string& pattern,
                                     const std::string& chromosome, const Position start, const Position end,
                                     size_t& count, std::string& msg)
    {
      if (pattern.empty()) {
        msg = "Motif can't be empty.";
        return false;
      }

      count = 0;

      if (chromosome != current_chromosome) {
        if (!load_sequence(chromosome, msg)) {
          return false;
        }
        current_chromosome = chromosome;
      }

      std::string sub = _sequence.substr(start, end - start);

      boost::regex e(pattern);
      boost::match_results<std::string::const_iterator> what;
      std::string::const_iterator begin_it = sub.begin();
      std::string::const_iterator end_it = sub.end();
      count = 0;
      while (boost::regex_search(begin_it, end_it, what, e)) {
        begin_it = what[0].second;
        ++count;
      }

      return true;
    }

    bool DatasetCache::get_sequence(const std::string& chromosome, const Position start, const Position end,
                                    std::string& sequence, std::string& msg)
    {
      if (chromosome != current_chromosome) {
        if (!load_sequence(chromosome, msg)) {
          return false;
        }
        current_chromosome = chromosome;
      }

      sequence = _sequence.substr(start, end - start);
      return true;
    }


    bool RunningCache::get_sequence(const std::string & norm_genome, const std::string & chromosome,
                                    const Position start, const Position end, std::string & sequence,
                                    StatusPtr status, std::string & msg)
    {
      if (caches.find(norm_genome) == caches.end()) {
        caches[norm_genome] = std::unique_ptr<DatasetCache>(new DatasetCache(norm_genome, status));
      }

      return caches[norm_genome]->get_sequence(chromosome, start, end, sequence, msg);
    }

    bool RunningCache::count_regions(const std::string & norm_genome, const std::string & chromosome, const std::string & pattern,
                                     const Position start, const Position end, size_t& count,
                                     StatusPtr status, std::string & msg)
    {
      if (caches.find(norm_genome) == caches.end()) {
        caches[norm_genome] = std::unique_ptr<DatasetCache>(new DatasetCache(norm_genome, status));
      }

      return caches[norm_genome]->count_regions(pattern, chromosome, start, end, count, msg);
    }
  }
}
