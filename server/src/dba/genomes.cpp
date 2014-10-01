//
//  genomes.cpp
//  epidb
//
//  Created by Felipe Albrecht on 04.04.14.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include <algorithm>
#include <string>
#include <map>

#include <boost/foreach.hpp>

#include "genomes.hpp"
#include "helpers.hpp"

#include "../extras/utils.hpp"

namespace epidb {
  namespace dba {
    namespace genomes {


      bool GenomeInfo::chromosome_size(const std::string &intern_chromosome, size_t &size, std::string &msg)
      {
        GenomeData::iterator it = data_.find(intern_chromosome);
        if (it != data_.end()) {
          size = it->second;
          return true;
        } else {
          msg = "Chromosome '" + intern_chromosome + "' not found.";
          return false;
        }
      }

      bool GenomeInfo::internal_chromosome(const std::string &chromosome, std::string &intern_chromosome, std::string &msg)
      {
        if ( chromosome.size() == 1 &&
             ((chromosome.compare("X") == 0) || (chromosome.compare("Y") == 0))) {
          intern_chromosome = std::string("chr") + chromosome;
        } else if (utils::is_number(chromosome)) {
          intern_chromosome = std::string("chr") + chromosome;
        } else {
          intern_chromosome = chromosome;
        }
        return true;
      }

      bool GenomeInfo::get_chromosome(const std::string &name, ChromosomeInfo &chromosome_info, std::string &msg)
      {
        GenomeData::iterator it = data_.find(name);
        if (it != data_.end()) {
          chromosome_info.name = it->first;
          chromosome_info.size = it->second;
          return true;
        }
        msg = "Chromosome " + name + " not found.";
        return false;
      }

      const std::vector<std::string> GenomeInfo::chromosomes()
      {
        std::vector<std::string> r;

        BOOST_FOREACH(const ChromosomeData &c, data_) {
          r.push_back(c.first);
        }
        return r;
      }

      bool get_chromosomes(const std::vector<std::string> &genomes,
                           std::set<std::string> &chromosomes, std::string &msg)
      {
        std::vector<std::string>::const_iterator it;
        for (it = genomes.begin(); it != genomes.end(); ++it) {
          if (!get_chromosomes(*it, chromosomes, msg)) {
            return false;
          }
        }
        return true;
      }

      bool get_chromosomes(const std::string &genome,
                           std::set<std::string> &chromosomes, std::string &msg)
      {
        GenomeInfoPtr genome_info;
        if (!get_genome_info(genome, genome_info, msg)) {
          return false;
        }
        std::vector<std::string> chroms = genome_info->chromosomes();
        BOOST_FOREACH(const std::string &chrom, chroms) {
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
        BOOST_FOREACH(const std::string & chrom, chroms) {
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
          std::stringstream ss;
          ss << "Genome '";
          ss << name;
          ss << "' not found";
          msg = ss.str();
          return false;
        }

        mongo::BSONObj o = result[0];
        std::vector<mongo::BSONElement> c = o["chromosomes"].Array();

        GenomeData data;
        for (std::vector<mongo::BSONElement>::iterator it = c.begin(); it != c.end(); it++) {
          mongo::BSONElement e = *it;
          std::string name = e["name"].String();
          int size = e["size"].Int();
          data[name] = size;
        }

        gi = boost::shared_ptr<GenomeInfo>(new GenomeInfo(name, data));
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