//
//  collections.cpp
//  epidb
//
//  Created by Felipe Albrecht on 09.05.2014
//  Copyright (c) 2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

#include "collections.hpp"

namespace epidb {
  namespace dba {

    const std::vector<std::string> Collections::build_valid_search_Collections()
    {
      std::vector<std::string> v;
      v.push_back(ANNOTATIONS());
      v.push_back(BIOSOURCES());
      v.push_back(COLUMN_TYPES());
      v.push_back(EPIGENETIC_MARKS());
      v.push_back(EXPERIMENTS());
      v.push_back(GENOMES());
      v.push_back(GENE_SETS());
      v.push_back(PROJECTS());
      v.push_back(SAMPLES());
      v.push_back(TECHNIQUES());
      v.push_back(TILINGS());
      return v;
    }

    const std::vector<std::string> &Collections::valid_search_Collections()
    {
      static std::vector<std::string> v = build_valid_search_Collections();
      return v;
    }

    bool Collections::is_valid_search_collection(const std::string &name)
    {
      return std::find(valid_search_Collections().begin(), valid_search_Collections().end(), name) != valid_search_Collections().end();
    }

    const bool Collections::get_collection_for_id(const std::string& id, std::string& collection) {
        unsigned int char_count = 0;
        for (const char& c : id) {
            if (isalpha(c)) {
                char_count++;
            }
        }
        std::string collection_prefix = id.substr(0, char_count);

        if ("a" == collection_prefix) {
            collection = ANNOTATIONS();
        } else if ("bs" == collection_prefix) {
            collection = BIOSOURCES();
        } else if ("ct" == collection_prefix) {
            collection = COLUMN_TYPES();
        } else if ("em" == collection_prefix) {
            collection = EPIGENETIC_MARKS();
        } else if ("gs" == collection_prefix) {
            collection = GENE_SETS();
        } else if ("e" == collection_prefix) {
            collection = EXPERIMENTS();
        } else if ("g" == collection_prefix) {
            collection = GENOMES();
        } else if ("p" == collection_prefix) {
            collection = PROJECTS();
        } else if ("q" == collection_prefix) {
            collection = QUERIES();
        } else if ("s" == collection_prefix) {
            collection = SAMPLES();
        } else if ("t" == collection_prefix) {
            collection = TECHNIQUES();
        } else if ("tr" == collection_prefix) {
            collection = TILINGS();
        } else if ("u" == collection_prefix) {
            collection = USERS();
        } else {
            return false;
        }
        return true;
    }

    const std::string &Collections::EXPERIMENTS()
    {
      static std::string experiments("experiments");
      return experiments;
    }

    const std::string &Collections::GENOMES()
    {
      static std::string genomes("genomes");
      return genomes;
    }

    const std::string &Collections::BIOSOURCES()
    {
      static std::string biosources("biosources");
      return biosources;
    }

    const std::string &Collections::BIOSOURCE_SYNONYMS()
    {
      static std::string biosource_synonyms("biosource_synonyms");
      return biosource_synonyms;
    }

    const std::string &Collections::BIOSOURCE_SYNONYM_NAMES()
    {
      static std::string biosource_synonyms_names("biosource_synonyms_names");
      return biosource_synonyms_names;
    }

    const std::string &Collections::BIOSOURCE_EMBRACING()
    {
      static std::string biosource_embracing("biosource_embracing");
      return biosource_embracing;
    }

    const std::string &Collections::EPIGENETIC_MARKS()
    {
      static std::string epigenetic_marks("epigenetic_marks");
      return epigenetic_marks;
    }

    const std::string &Collections::REGIONS()
    {
      static std::string regions("regions");
      return regions;
    }

    const std::string &Collections::ANNOTATIONS()
    {
      static std::string annotations("annotations");
      return annotations;
    }

    const std::string &Collections::GENE_SETS()
    {
      static std::string genomes("gene_sets");
      return genomes;
    }


    const std::string &Collections::QUERIES()
    {
      static std::string queries("queries");
      return queries;
    }

    const std::string &Collections::SAMPLES()
    {
      static std::string samples("samples");
      return samples;
    }

    const std::string &Collections::SEQUENCES()
    {
      static std::string sequences("sequences");
      return sequences;
    }

    const std::string &Collections::TECHNIQUES()
    {
      static std::string techniques("techniques");
      return techniques;
    }

    const std::string &Collections::TILINGS()
    {
      static std::string tilings("tilings");
      return tilings;
    }

    const std::string &Collections::COLUMN_TYPES()
    {
      static std::string column_types("column_types");
      return column_types;
    }

    const std::string &Collections::PROJECTS()
    {
      static std::string projects("projects");
      return projects;
    }

    const std::string &Collections::USERS()
    {
      static std::string users("users");
      return users;
    }

    const std::string &Collections::WEB_ACCESS()
    {
      static std::string users("web_access");
      return users;
    }

    const std::string &Collections::TEXT_SEARCH()
    {
      static std::string tex_search("text_search");
      return tex_search;
    }

    const std::string &Collections::SETTINGS()
    {
      static std::string settings("settings");
      return settings;
    }

    const std::string &Collections::COUNTERS()
    {
      static std::string counters("counters");
      return counters;
    }

    const std::string &Collections::KEY_MAPPER()
    {
      static std::string key_mapper("key_mapper");
      return key_mapper;
    }

    const std::string &Collections::JOBS()
    {
      static std::string jobs("jobs");
      return jobs;
    }

    const std::string &Collections::PROCESSING()
    {
      static std::string settings("processing");
      return settings;
    }

    const std::string &Collections::PROCESSING_OPS()
    {
      static std::string settings("processing_ops");
      return settings;
    }

  }
}
