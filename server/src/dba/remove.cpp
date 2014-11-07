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
          msg = "Annotation " + id + " not found";
          return false;
        }

        int dataset_id = annotation[KeyMapper::DATASET()].Int();
        std::string genome_name = annotation["norm_genome"].String();

        // find others annotations with the same id
        std::vector<mongo::BSONObj> results;
        if (!helpers::get(Collections::ANNOTATIONS(), BSON(KeyMapper::DATASET() << dataset_id), results, msg)) {
          return false;
        }

        // keep it if it has more than one experiment pointing to this dataset
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

      bool experiment(const std::string &id, const std::string &user_id, std::string &msg)
      {
        mongo::BSONObj experiment;
        if (!data::experiment(id, experiment, msg)) {
          msg = "Experiment " + id + " not found";
          return false;
        }

        int dataset_id = experiment[KeyMapper::DATASET()].Int();
        std::string genome_name = experiment["norm_genome"].String();

        // find others experiments with the same id
        std::vector<mongo::BSONObj> results;
        if (!helpers::get(Collections::EXPERIMENTS(), BSON(KeyMapper::DATASET() << dataset_id), results, msg)) {
          return false;
        }

        // keep it if it has more than one experiment pointing to this dataset
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
        if (!helpers::remove_one(helpers::collection_name(Collections::EXPERIMENTS()), id, msg)) {
          return false;
        }

        return true;
      }

      bool genome(const std::string &id, const std::string &user_id, std::string &msg)
      {
        mongo::BSONObj genome;
        if (!data::genome(id, genome, msg)) {
          msg = "Genome " + id + " not found";
          return false;
        }

        const std::string genome_name = genome["norm_name"].String();

        // Check if some experiment is still using this genome
        std::vector<mongo::BSONObj> experiments;
        if (!helpers::get(Collections::EXPERIMENTS(), "norm_genome", genome_name, experiments, msg)) {
          return false;
        }
        if (!experiments.empty()) {
          msg = "This genome is being used by experiments.";
          return false;
        }

        // Check if some annotations is still using this genome
        std::vector<mongo::BSONObj> annotations;
        std::cerr << genome_name << std::endl;
        if (!helpers::get(Collections::ANNOTATIONS(), "norm_genome", genome_name, annotations, msg)) {
          return false;
        }
        if (annotations.size() > 1) {
          //  (TODO: show experiments)
          msg = "This genome is being used by annotations.";
          return false;
        }

        std::cerr << annotations.size() << std::endl;
        if (annotations.size() == 1) {
          std::string own_annotation_name = annotations[0]["norm_name"].String();
          if (own_annotation_name != genome_name) {
            //  (TODO: show experiments)
            msg = "This genome is being used by annotations.";
            return false;
          }
          std::string own_annotation_id = annotations[0]["_id"].String();
          std::cerr << own_annotation_id << std::endl;
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
        mongo::BSONObj project;
        if (!data::project(id, project, msg)) {
          msg = "Project " + id + " not found";
          return false;
        }

        const std::string project_name = project["norm_name"].String();

        // Check if some experiment is still using this project
        std::vector<mongo::BSONObj> experiments;
        if (!helpers::get(Collections::EXPERIMENTS(), "norm_project", project_name, experiments, msg)) {
          return false;
        }
        if (!experiments.empty()) {
          //  (TODO: show experiments)
          msg = "This project is being used by experiments.";
          return false;
        }

        // Start deleting the data
        // delete from full text search
        if (!search::remove(id, msg)) {
          return false;
        }

        // Delete project from projects collection
        if (!helpers::remove_one(helpers::collection_name(Collections::PROJECTS()), id, msg)) {
          return false;
        }

        return true;
      }

      bool biosource(const std::string &id, const std::string &user_id, std::string &msg)
      {
        mongo::BSONObj biosource;
        if (!data::biosource(id, biosource, msg)) {
          msg = "Biosource " + id + " not found";
          return false;
        }

        const std::string biosource_name = biosource["norm_name"].String();

        // Check if some experiment is still using this project
        std::vector<mongo::BSONObj> samples;
        if (!helpers::get(Collections::SAMPLES(), "norm_biosource_name", biosource_name, samples, msg)) {
          return false;
        }
        if (!samples.empty()) {
          //  (TODO: show experiments)
          msg = "This biosource is being used by samples.";
          return false;
        }

        // Start deleting the data
        // delete from full text search
        if (!search::remove(id, msg)) {
          return false;
        }

        // Delete project from projects collection

        // TODO: check relationship
        //   if is leaf, okay, otherwise error
        //   remove from search
        // TODO: check synonyms
        //    remove if is a synonyms?
        //    synonym ownership tracking (another collection)
        if (!helpers::remove_one(helpers::collection_name(Collections::BIOSOURCES()), id, msg)) {
          return false;
        }

        return true;
      }

      bool sample(const std::string &id, const std::string &user_id, std::string &msg)
      {
        mongo::BSONObj sample;
        if (!data::sample(id, sample, msg)) {
          msg = "Sample " + id + " not found";
          return false;
        }

        const std::string sample_id = sample["_id"].String();

        // Check if some experiment is still using this project
        std::vector<mongo::BSONObj> experiments;
        if (!helpers::get(Collections::EXPERIMENTS(), "sample_id", sample_id, experiments, msg)) {
          return false;
        }
        if (!experiments.empty()) {
          //  (TODO: show experiments)
          msg = "This sample is being used by experiments.";
          return false;
        }

        // Start deleting the data
        // delete from full text search
        if (!search::remove(id, msg)) {
          return false;
        }

        // Delete sample from samples collection
        if (!helpers::remove_one(helpers::collection_name(Collections::SAMPLES()), id, msg)) {
          return false;
        }

        return true;
      }

      bool epigenetic_mark(const std::string &id, const std::string &user_id, std::string &msg)
      {
        mongo::BSONObj epigenetic_mark;
        if (!data::epigenetic_mark(id, epigenetic_mark, msg)) {
          msg = "Epigenetic Mark " + id + " not found";
          return false;
        }

        const std::string epigenetic_mark_name = epigenetic_mark["norm_name"].String();

        // Check if some experiment is still using this project
        std::vector<mongo::BSONObj> experiments;
        if (!helpers::get(Collections::EXPERIMENTS(), "norm_epigenetic_mark", epigenetic_mark_name, experiments, msg)) {
          return false;
        }
        if (!experiments.empty()) {
          //  (TODO: show experiments)
          msg = "This epigenetic mark is being used by experiments.";
          return false;
        }

        // Start deleting the data
        // delete from full text search
        if (!search::remove(id, msg)) {
          return false;
        }

        // Delete epigenetic mark from epigenetic marks collection
        if (!helpers::remove_one(helpers::collection_name(Collections::EPIGENETIC_MARKS()), id, msg)) {
          return false;
        }

        return true;
      }

      bool technique(const std::string &id, const std::string &user_id, std::string &msg)
      {
        mongo::BSONObj technique;
        if (!data::technique(id, technique, msg)) {
          msg = "Technique " + id + " not found";
          return false;
        }

        const std::string technique_name = technique["norm_name"].String();

        // Check if some experiment is still using this project
        std::vector<mongo::BSONObj> experiments;
        if (!helpers::get(Collections::EXPERIMENTS(), "norm_technique", technique_name, experiments, msg)) {
          return false;
        }
        if (!experiments.empty()) {
          //  (TODO: show experiments)
          msg = "This technique is being used by experiments.";
          return false;
        }

        // Start deleting the data
        // delete from full text search
        if (!search::remove(id, msg)) {
          return false;
        }

        // Delete epigenetic mark from epigenetic marks collection
        if (!helpers::remove_one(helpers::collection_name(Collections::TECHNIQUES()), id, msg)) {
          return false;
        }

        return true;
      }

      bool query(const std::string &id, const std::string &user_id, std::string &msg)
      {
        return false;
      }

      bool tiling_region(const std::string &id, const std::string &user_id, std::string &msg)
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
