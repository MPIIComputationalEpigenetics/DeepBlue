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
      v.push_back(BIO_SOURCES());
      v.push_back(EPIGENETIC_MARKS());
      v.push_back(EXPERIMENTS());
      v.push_back(GENOMES());
      v.push_back(PROJECTS());
      v.push_back(SAMPLES());
      v.push_back(SAMPLE_FIELDS());
      v.push_back(TECHNIQUES());
      v.push_back(TILINGS());
      return v;
    }

    const std::vector<std::string> &Collections::valid_search_Collections()
    {
      static std::vector<std::string> v = build_valid_search_Collections();
      return v;
    }

    bool Collections::is_valid_search_collection(std::string &name)
    {
      return std::find(valid_search_Collections().begin(), valid_search_Collections().end(), name) != valid_search_Collections().end();
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

    const std::string &Collections::BIO_SOURCES()
    {
      static std::string bio_sources("bio_sources");
      return bio_sources;
    }

    const std::string &Collections::BIO_SOURCE_SYNONYMS()
    {
      static std::string bio_source_synonyms("bio_source_synonyms");
      return bio_source_synonyms;
    }

    const std::string &Collections::BIO_SOURCE_SYNONYM_NAMES()
    {
      static std::string bio_source_synonyms_names("bio_source_synonyms.names");
      return bio_source_synonyms_names;
    }

    const std::string &Collections::BIO_SOURCE_EMBRACING()
    {
      static std::string bio_source_embracing("bio_source_embracing");
      return bio_source_embracing;
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

    const std::string &Collections::SAMPLE_FIELDS()
    {
      static std::string samples_fields("samples.fields");
      return samples_fields;
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
  }
}
