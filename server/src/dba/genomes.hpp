//
//  genomes.hpp
//  epidb
//
//  Created by Felipe Albrecht on 04.04.14.
//  Copyright (c) 2014 Max Planck Institute for Computer Science. All rights reserved.
//

#ifndef EPIDB_DBA_GENOMES_HPP
#define EPIDB_DBA_GENOMES_HPP

#include <sstream>
#include <unordered_map>

#include <mongo/bson/bson.h>

namespace epidb {
  namespace dba {
    namespace genomes {

      typedef std::pair<std::string, size_t> ChromosomeData;
      typedef std::unordered_map<std::string, size_t> GenomeData;
      typedef std::unordered_map<std::string, std::string> NamesPairs;


      struct ChromosomeInfo {
        std::string name;
        size_t size;

        ChromosomeInfo() {}
        ChromosomeInfo(std::string n, size_t s) :
          name(n), size(s) {}

        inline bool operator< (const ChromosomeInfo &b) const
        {
          return this->name.compare(b.name) < 0;
        }
      };

      class GenomeInfo {
      private:
        std::string name_;
        GenomeData data_;
        NamesPairs names_pair_;

      public:
        GenomeInfo(const std::string &name, GenomeData data, NamesPairs names_pair)
          : name_(name), data_(data), names_pair_(names_pair)
        {}

        bool internal_chromosome(const std::string &chromosome, std::string &intern_chromosome, std::string &msg);
        bool chromosome_size(const std::string &chromosome, size_t &size, std::string &msg);

        bool get_chromosome(const std::string &name, ChromosomeInfo &chromosome_info, std::string &msg);
        const std::vector<std::string> chromosomes();
      };

      typedef boost::shared_ptr<GenomeInfo> GenomeInfoPtr;

      bool get_chromosomes(const std::string &genome,
                           std::set<std::string> &chromosomes, std::string &msg);

      bool get_chromosomes(const std::vector<std::string> &genomes,
                           std::set<std::string> &chromosomes, std::string &msg);

      bool get_chromosomes(const std::string &genome,
                           std::vector<ChromosomeInfo> &chromosomes, std::string &msg);

      bool get_genome_info(const std::string &name, GenomeInfoPtr &, std::string &msg);

      bool chromosome_size(const std::string &genome, const std::string &chromosome, size_t &size, std::string &msg);
    }
  }
}

#endif