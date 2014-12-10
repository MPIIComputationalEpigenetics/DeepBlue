//
//  data.cpp
//  epidb
//
//  Created by Felipe Albrecht on 06.11.14.
//  Copyright (c) 2014 Max Planck Institute for Computer Science. All rights reserved.
//
#include <string>

#include "collections.hpp"
#include "column_types.hpp"
#include "helpers.hpp"

namespace epidb {
  namespace dba {
    namespace data {

      bool sample(const std::string &id, mongo::BSONObj &result, std::string &msg)
      {
        if (helpers::get_one(Collections::SAMPLES(), mongo::Query(BSON("_id" << id)), result, msg)) {
          return true;
        } else {
          msg = "Sample ID '" + id + "' not found.";
          return false;
        }
      }

      bool genome(const std::string &id, mongo::BSONObj &result, std::string &msg)
      {
        if (helpers::get_one(Collections::GENOMES(), mongo::Query(BSON("_id" << id)), result, msg)) {
          return true;
        } else {
          msg = "Genome ID '" + id + "' not found.";
          return false;
        }
      }

      bool project(const std::string &id, mongo::BSONObj &result, std::string &msg)
      {
        if (helpers::get_one(Collections::PROJECTS(), mongo::Query(BSON("_id" << id)), result, msg)) {
          return true;
        } else {
          msg = "Project ID '" + id + "' not found.";
          return false;
        }
      }

      bool technique(const std::string &id, mongo::BSONObj &result, std::string &msg)
      {
        if (helpers::get_one(Collections::TECHNIQUES(), mongo::Query(BSON("_id" << id)), result, msg)) {
          return true;
        } else {
          msg = "Technique ID '" + id + "' not found.";
          return false;
        }
      }

      bool biosource(const std::string &id, mongo::BSONObj &result, std::string &msg)
      {
        if (helpers::get_one(Collections::BIOSOURCES(), mongo::Query(BSON("_id" << id)), result, msg)) {
          return true;
        } else {
          msg = "BioSource ID '" + id + "' not found.";
          return false;
        }
      }

      bool epigenetic_mark(const std::string &id, mongo::BSONObj &result, std::string &msg)
      {
        if (helpers::get_one(Collections::EPIGENETIC_MARKS(), mongo::Query(BSON("_id" << id)), result, msg)) {
          return true;
        } else {
          msg = "Epigenetic Mark ID '" + id + "' not found.";
          return false;
        }
      }

      bool annotation(const std::string &id, mongo::BSONObj &result, std::string &msg)
      {
        if (helpers::get_one(Collections::ANNOTATIONS(), mongo::Query(BSON("_id" << id)), result, msg)) {
          return true;
        } else {
          msg = "Annotation ID '" + id + "' not found.";
          return false;
        }
      }

      bool experiment(const std::string &id, mongo::BSONObj &result, std::string &msg)
      {
        if (helpers::get_one(Collections::EXPERIMENTS(), mongo::Query(BSON("_id" << id)), result, msg)) {
          return true;
        } else {
          msg = "Experiment ID '" + id + "' not found.";
          return false;
        }
      }

      bool query(const std::string &id, mongo::BSONObj &result, std::string &msg)
      {
        if (helpers::get_one(Collections::QUERIES(), mongo::Query(BSON("_id" << id)), result, msg)) {
          return true;
        } else {
          msg = "Query ID '" + id + "' not found.";
          return false;
        }
      }

      bool sample_field(const std::string &id, mongo::BSONObj &result, std::string &msg)
      {
        if (helpers::get_one(Collections::SAMPLE_FIELDS(), mongo::Query(BSON("_id" << id)), result, msg)) {
          return true;
        } else {
          msg = "Experiment ID '" + id + "' not found.";
          return false;
        }
      }

      bool tiling_region(const std::string &id, mongo::BSONObj &result, std::string &msg)
      {
        if (helpers::get_one(Collections::TILINGS(), mongo::Query(BSON("_id" << id)), result, msg)) {
          return true;
        } else {
          msg = "Tiling Regions ID '" + id + "' not found.";
          return false;
        }
      }

      bool column_type(const std::string &id, mongo::BSONObj &result, std::string &msg)
      {
        if (!helpers::get_one(Collections::COLUMN_TYPES(), mongo::Query(BSON("_id" << id)),  result, msg)) {
          return false;
        } else {
          msg = "Column type ID " + id + " not found";
          return false;
        }
      }

    }
  }
}