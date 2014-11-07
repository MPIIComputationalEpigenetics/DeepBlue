//
//  delete.cpp
//  epidb
//
//  Created by Felipe Albrecht on 06.11.14.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include <string>

#include <mongo/bson/bson.h>

#include "collections.hpp"
#include "data.hpp"
#include "genomes.hpp"
#include "helpers.hpp"
#include "key_mapper.hpp"
#include "remove.hpp"
#include "full_text.hpp"

namespace epidb {
  namespace dba {
    namespace remove {

      bool dataset(const int dataset_id, const std::string &genome_name, std::string &msg)
      {
        std::set<std::string> chromosomes;
        if (!genomes::get_chromosomes(genome_name, chromosomes, msg)) {
          return false;
        }

        // Check if the genome regions collections are empty.
        for (const std::string &internal_chromosome : chromosomes ) {
          std::string collection = helpers::region_collection_name(genome_name, internal_chromosome);
          if (!helpers::remove_all(collection, BSON(KeyMapper::DATASET() << dataset_id), msg)) {
            return false;
          }
        }

        return true;
      }

      bool annotation(const std::string &id, const std::string &user_id, std::string &msg)
      {
        mongo::BSONObj annotation;
        if (!data::annotation(id, annotation, msg)) {
          return false;
        }

        int dataset_id = annotation[KeyMapper::DATASET()].Int();
        std::string genome_name = annotation["genome"].String();

        // find others annotations with the same id
        std::vector<mongo::BSONObj> results;
        if (!helpers::get(Collections::ANNOTATIONS(), BSON(KeyMapper::DATASET() << dataset_id), results, msg)) {
          return false;
        }

        // if have more than one, keep the dataset
        // if have one, remove dataset
        if (results.size() == 1) {
          if (!remove::dataset(dataset_id, genome_name, msg)) {
            return false;
          }
        }

        // delete from full text search
        if (!search::remove(id, msg)) {
          return false;
        }

        // Delete from collection
        if (!helpers::remove_one(helpers::collection_name(Collections::ANNOTATIONS()), id, msg)) {
          return false;
        }

        return true;
      }

      bool genome(const std::string &id, const std::string &user_id, std::string &msg)
      {
        mongo::BSONObj genome;
        if (!data::genome(id, genome, msg)) {
          return false;
        }

        const std::string genome_name = genome["norm_name"].String();

        // Check if some experiment is still using this genome
        std::vector<mongo::BSONObj> experiments;

        if (!helpers::get(Collections::EXPERIMENTS(), "genome", genome_name, experiments, msg)) {
          return false;
        }
        if (!experiments.empty()) {
          //  (TODO: show experiments)
          msg = "Some experiments are still using this genome.";
          return false;
        }

        // Check if some annotations is still using this genome
        // TODO: delete the automatically created annotation
        std::vector<mongo::BSONObj> annotations;
        std::cerr << genome_name << std::endl;
        if (!helpers::get(Collections::ANNOTATIONS(), "genome", genome_name, annotations, msg)) {
          return false;
        }
        if (annotations.size() > 1) {
          //  (TODO: show experiments)
          msg = "Some annotations are still using this genome.";
          return false;
        }

        std::cerr << annotations.size() << std::endl;
        if (annotations.size() == 1) {
          std::string own_annotation_name = annotations[0]["name"].String();
          if (own_annotation_name != genome_name) {
            //  (TODO: show experiments)
            msg = "Some annotations are still using this genome.";
            return false;
          }
          std::string own_annotation_id = annotations[0]["_id"].String();
          if (!remove::annotation(own_annotation_id, user_id, msg)) {
            return false;
          }
        }

        std::set<std::string> chromosomes;
        if (!genomes::get_chromosomes(genome_name, chromosomes, msg)) {
          return false;
        }

        // Check if the genome regions collections are empty.
        for (const std::string &internal_chromosome : chromosomes ) {
          std::string collection = helpers::region_collection_name(genome_name, internal_chromosome);
          unsigned long long size;
          if (!helpers::collection_size(collection, size, msg)) {
            std::cerr << "false " << std::endl;
            return false;
          }
          if (size > 0) {
            msg = "FATAL ERROR WHILE DELETING GENOME " + id + ". Collection " + collection + " is not empty. Please, contact the developers";
            return false;
          }
        }

        // Start deleting the data

        // delete from full text search
        if (!search::remove(id, msg)) {
          return false;
        }

        // For chromosome, remove the collection
        // Check if the genome regions collections are empty.
        for (const std::string &internal_chromosome : chromosomes ) {
          std::string collection = helpers::region_collection_name(genome_name, internal_chromosome);
          if (!helpers::remove_collection(collection, msg)) {
            return false;
          }
        }

        // Delete genome from genomes collection
        if (!helpers::remove_one(helpers::collection_name(Collections::GENOMES()), id, msg)) {
          return false;
        }

        return true;
      }

      bool project(const std::string &id, const std::string &user_id, std::string &msg)
      {
        return false;
      }

      bool biosource(const std::string &id, const std::string &user_id, std::string &msg)
      {
        return false;
      }

      bool sample_by_id(const std::string &id, const std::string &user_id, std::string &msg)
      {
        return false;
      }

      bool epigenetic_mark(const std::string &id, const std::string &user_id, std::string &msg)
      {
        return false;
      }

      bool experiment(const std::string &id, const std::string &user_id, std::string &msg)
      {
        return false;
      }

      bool query(const std::string &id, const std::string &user_id, std::string &msg)
      {
        return false;
      }

      bool tiling_region(const std::string &id, const std::string &user_id, std::string &msg)
      {
        return false;
      }

      bool technique(const std::string &id, const std::string &user_id, std::string &msg)
      {
        return false;
      }

      bool sample_field(const std::string &id, const std::string &user_id, std::string &msg)
      {
        return false;
      }

      bool column_type(const std::string &id, const std::string &user_id, std::string &msg)
      {
        return false;
      }
    }
  }
}
