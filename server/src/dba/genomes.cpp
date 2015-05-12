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

#include "genomes.hpp"
#include "helpers.hpp"

#include "../extras/utils.hpp"

namespace epidb {
  namespace dba {
    namespace genomes {


      bool GenomeInfo::chromosome_size(const std::string &intern_chromosome, size_t &size, std::string &msg) const
      {
        GenomeData::const_iterator it = data_.find(intern_chromosome);
        if (it != data_.end()) {
          size = it->second;
          return true;
        } else {
          msg = "Chromosome '" + intern_chromosome + "' not found.";
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
        if ((chromosome.size() > 3) && (chromosome.compare(0, 3, "chr") == 0)) {
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

        msg = "Chromosome " + chromosome + " not found in the genome assembly " + name_;
        return false;

      }

      bool GenomeInfo::get_chromosome(const std::string &name, ChromosomeInfo &chromosome_info, std::string &msg) const
      {
        GenomeData::const_iterator it = data_.find(name);
        if (it != data_.end()) {
          chromosome_info.name = it->first;
          chromosome_info.size = it->second;
          return true;
        }
        msg = "Chromosome " + name + " not found.";
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
        for (const auto &genome: genomes) {
          if (!get_chromosomes(genome, chromosomes, msg)) {
            return false;
          }
        }
        return true;
      }

      bool get_chromosomes(const std::set<std::string> &genomes,
                           std::set<std::string> &chromosomes, std::string &msg) {
        return get_chromosomes<std::set<std::string>>(genomes, chromosomes, msg);
      }

      bool get_chromosomes(const std::vector<std::string> &genomes,
                           std::set<std::string> &chromosomes, std::string &msg) {
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

      bool get_genome_info(const std::string &name, GenomeInfoPtr &gi, std::string &msg)
      {
        std::string norm_genome = utils::normalize_name(name);

        std::vector<mongo::BSONObj> result;
        if (!helpers::get("genomes", "norm_name", norm_genome, result, msg)) {
          return false;
        }

        if (result.size() == 0) {
          msg = "Genome '" + name +  "' not found";
          return false;
        }

        mongo::BSONObj o = result[0];
        std::vector<mongo::BSONElement> c = o["chromosomes"].Array();

        GenomeData data;
        NamesPairs names_pairs;
        for (std::vector<mongo::BSONElement>::iterator it = c.begin(); it != c.end(); it++) {
          mongo::BSONElement e = *it;
          std::string name = e["name"].String();
          std::string norm_name = utils::normalize_name(name);
          int size = e["size"].Int();
          data[name] = size;
          names_pairs[norm_name] = name;
        }

        gi = boost::shared_ptr<GenomeInfo>(new GenomeInfo(name, data, names_pairs));
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