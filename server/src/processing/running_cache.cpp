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
#include <regex>
#include <string>

#include "../dba/genomes.hpp"
#include "../dba/retrieve.hpp"
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
      return _sequence_retriever.retrieve(_genome, chromosome, 0, size, _sequence, msg);
    }

    bool DatasetCache::count_regions(const std::string& pattern,
                                     const std::string& chromosome, const Position start, const Position end,
                                     size_t& count, std::string& msg)
    {
      count = 0;

      if (chromosome != current_chromosome) {
        if (!load_sequence(chromosome, msg)) {
          return false;
        }
        current_chromosome = chromosome;
      }

      std::regex word_regex;
      try {
        std::regex word_regex(pattern, std::regex::egrep);
        std::string sub = _sequence.substr(start, end - start);
        auto words_begin = std::sregex_iterator(sub.begin(), sub.end(), word_regex);
        auto words_end = std::sregex_iterator();
        count = std::distance(words_begin, words_end);
      } catch (const std::regex_error& e) {
        msg = "Your motif '" + pattern + "' has an error: " + e.what();
        return false;
      }

      return true;
    }

    bool RunningCache::count_regions(const std::string& genome, const std::string& chromosome, const std::string& pattern,
                                     const Position start, const Position end, size_t& count,
                                     StatusPtr status, std::string& msg)
    {
      if (caches.find(genome) == caches.end()) {
        caches[genome] = std::unique_ptr<DatasetCache>(new DatasetCache(genome, status));
      }

      return caches[genome]->count_regions(pattern, chromosome, start, end, count, msg);
    }
  }
}
