//
//  collections.hpp
//  epidb
//
//  Created by Felipe Albrecht on 09.05.2014
//  Copyright (c) 2014 Max Planck Institute for Computer Science. All rights reserved.
//

#ifndef EPIDB_DBA_COLLECTIONS_HPP
#define EPIDB_DBA_COLLECTIONS_HPP

#include <string>
#include <vector>

namespace epidb {
  namespace dba {
    class Collections {
    private:
      static const std::vector<std::string> build_valid_search_Collections();

    public:
      static bool is_valid_search_collection(const std::string &name);

      static const std::vector<std::string> &valid_search_Collections();

      static const bool get_collection_for_id(const std::string& id, std::string& collection);

      static const std::string &EXPERIMENTS();
      static const std::string &GENOMES();
      static const std::string &BIOSOURCES();
      static const std::string &BIOSOURCE_SYNONYMS();
      static const std::string &BIOSOURCE_SYNONYM_NAMES();
      static const std::string &BIOSOURCE_EMBRACING();
      static const std::string &EPIGENETIC_MARKS();
      static const std::string &REGIONS();
      static const std::string &ANNOTATIONS();
      static const std::string &GENE_SETS();
      static const std::string &GENES();
      static const std::string &QUERIES();
      static const std::string &SAMPLES();
      static const std::string &SEQUENCES();
      static const std::string &TECHNIQUES();
      static const std::string &TILINGS();

      static const std::string &COLUMN_TYPES();

      static const std::string &PROJECTS();
      static const std::string &USERS();
      static const std::string &WEB_ACCESS();
      static const std::string &TEXT_SEARCH();
      static const std::string &SETTINGS();
      static const std::string &COUNTERS();
      static const std::string &KEY_MAPPER();


      // Jobs management
      static const std::string &JOBS();
      static const std::string &PROCESSING();
      static const std::string &PROCESSING_OPS();
    };
  }
}

#endif
