//
//  genomes.hpp
//  DeepBlue Epigenomic Data Server

//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.

//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.

//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
//  Created by Felipe Albrecht on 04.04.14.
//  Copyright (c) 2016 Max Planck Institute for Informatics. All rights reserved.

//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.

//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.

//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
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

        ChromosomeInfo():
          name(), size(0) {}
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

        bool internal_chromosome(const std::string &chromosome, std::string &intern_chromosome, std::string &msg) const;
        bool chromosome_size(const std::string &chromosome, size_t &size, std::string &msg) const ;

        bool get_chromosome(const std::string &name, ChromosomeInfo &chromosome_info, std::string &msg) const;
        const std::vector<std::string> chromosomes() const;
      };

      typedef std::shared_ptr<GenomeInfo> GenomeInfoPtr;

      bool get_chromosomes(const std::string &genome,
                           std::set<std::string> &chromosomes, std::string &msg);

      bool get_chromosomes(const std::set<std::string> &genomes,
                           std::set<std::string> &chromosomes, std::string &msg);

      bool get_chromosomes(const std::vector<std::string> &genomes,
                           std::set<std::string> &chromosomes, std::string &msg);

      bool get_chromosomes(const std::string &genome,
                           std::vector<ChromosomeInfo> &chromosomes, std::string &msg);

      bool get_genome_info(const std::string &id_name, GenomeInfoPtr &, std::string &msg);

      bool chromosome_size(const std::string &genome, const std::string &chromosome, size_t &size, std::string &msg);
    }
  }
}

#endif