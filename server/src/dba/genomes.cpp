//
//  genomes.cpp
//  epidb
//
//  Created by Felipe Albrecht on 04.04.14.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include <algorithm>
#include <ctype.h>
#include <functional>
#include <iostream>
#include <string>
#include <utility>

#include <boost/foreach.hpp>

#include "collections.hpp"
#include "genomes.hpp"
#include "helpers.hpp"

#include "../extras/utils.hpp"

#include "../errors.hpp"

namespace epidb {
  namespace dba {
    namespace genomes {
      bool GenomeInfo::chromosome_size(const std::string &intern_chromosome, size_t &size, std::string &msg) const
      {
        auto it = data_.find(intern_chromosome);
        if (it != data_.end()) {
          size = it->second;
          return true;
        } else {
          msg = Error::m(ERR_INVALID_CHROMOSOME_NAME, intern_chromosome);
          return false;
        }
      }

      bool GenomeInfo::internal_chromosome(const std::string &chromosome, std::string &intern_chromosome, std::string &msg) const
      {
        if (chromosome.empty()) {
          msg = "The chromosome name is empty";
          return false;
        }

        std::string norm_chromosome = utils::normalize_name(chromosome);

        NamesPairs::const_iterator p;

        // Return if the name is correct
        p = names_pair_.find(norm_chromosome);
        if (p != names_pair_.end()) {
          intern_chromosome = p->second;
          return true;
        }

        // Check if the name contains extra "chr". Remove it and check.
        if ((norm_chromosome.size() > 3) && (norm_chromosome.compare(0, 3, "chr") == 0)) {
          std::string tmp = norm_chromosome.substr(3);
          p = names_pair_.find(tmp);
          if (p != names_pair_.end()) {
            intern_chromosome = p->second;
            return true;
          }
        }

        // Final try: Include "chr" in front of the chromosome name.
        std::string tmp = std::string("chr") + norm_chromosome;
        p = names_pair_.find(tmp);
        if (p != names_pair_.end()) {
          intern_chromosome = p->second;
          return true;
        }

        msg = Error::m(ERR_INVALID_CHROMOSOME_NAME_GENOME, chromosome, name_);
        return false;
      }

      bool GenomeInfo::get_chromosome(const std::string &name, ChromosomeInfo &chromosome_info, std::string &msg) const
      {
        std::string norm_chromosome = utils::normalize_name(name);
        auto it = data_.find(name);
        if (it != data_.end()) {
          chromosome_info.name = it->first;
          chromosome_info.size = it->second;
          return true;
        }
        msg = Error::m(ERR_INVALID_CHROMOSOME_NAME, norm_chromosome);
        return false;
      }

      const std::vector<std::string> GenomeInfo::chromosomes() const
      {
        std::vector<std::string> r;

        for (const auto &c : data_) {
          r.push_back(c.first);
        }
        return r;
      }

      template <class T>
      bool get_chromosomes(const T &genomes,
                           std::set<std::string> &chromosomes, std::string &msg)
      {
        for (const auto &genome : genomes) {
          if (!get_chromosomes(genome, chromosomes, msg)) {
            return false;
          }
        }
        return true;
      }

      bool get_chromosomes(const std::set<std::string> &genomes,
                           std::set<std::string> &chromosomes, std::string &msg)
      {
        return get_chromosomes<std::set<std::string>>(genomes, chromosomes, msg);
      }

      bool get_chromosomes(const std::vector<std::string> &genomes,
                           std::set<std::string> &chromosomes, std::string &msg)
      {
        return get_chromosomes<std::vector<std::string>>(genomes, chromosomes, msg);
      }

      bool get_chromosomes(const std::string &genome,
                           std::set<std::string> &chromosomes, std::string &msg)
      {
        GenomeInfoPtr genome_info;
        if (!get_genome_info(genome, genome_info, msg)) {
          return false;
        }
        std::vector<std::string> chroms = genome_info->chromosomes();
        for (const auto &chrom : chroms) {
          chromosomes.insert(chrom);
        }
        return true;
      }

      bool get_chromosomes(const std::string &genome,
                           std::vector<ChromosomeInfo> &chromosomes, std::string &msg)
      {
        GenomeInfoPtr genome_info;
        if (!get_genome_info(genome, genome_info, msg)) {
          return false;
        }
        std::vector<std::string> chroms = genome_info->chromosomes();
        for (const auto &chrom : chroms) {
          ChromosomeInfo chromosome_info;
          if (!genome_info->get_chromosome(chrom, chromosome_info, msg)) {
            return false;
          }
          chromosomes.push_back(chromosome_info);
        }
        std::sort(chromosomes.begin(), chromosomes.end());

        return true;
      }

      bool get_genome_info(const std::string &id_name, GenomeInfoPtr &gi, std::string &msg)
      {
        mongo::BSONObj result;

        if (utils::is_id(id_name, "g")) {
          if (!helpers::get_one(Collections::GENOMES(), BSON("_id" << id_name), result)) {
            msg = Error::m(ERR_INVALID_GENOME_ID, id_name);
            return false;
          }
        } else {
          std::string norm_genome = utils::normalize_name(id_name);
          if (!helpers::get_one(Collections::GENOMES(), BSON("norm_name" << norm_genome), result)) {
            msg = Error::m(ERR_INVALID_GENOME_NAME, id_name);
            return false;
          }
        }

        std::vector<mongo::BSONElement> c = result["chromosomes"].Array();
        std::string genome_name = result["name"].String();

        GenomeData data;
        NamesPairs names_pairs;
        for (auto&  e : c) {
          std::string name = e["name"].String();
          std::string norm_name = utils::normalize_name(name);
          int size = e["size"].Int();
          data[name] = size;
          names_pairs[norm_name] = name;
        }

        gi = std::shared_ptr<GenomeInfo>(new GenomeInfo(genome_name, data, names_pairs));
        return true;
      }

      bool chromosome_size(const std::string &genome, const std::string &chromosome, size_t &size, std::string &msg)
      {
        GenomeInfoPtr gi;
        if (!get_genome_info(genome,  gi, msg)) {
          return false;
        }

        ChromosomeInfo chromosome_info;
        if (!gi->get_chromosome(chromosome, chromosome_info, msg)) {
          return false;
        }

        size = chromosome_info.size;
        return true;
      }
    }
  }
}