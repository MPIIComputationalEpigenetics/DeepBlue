//
//  genes.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 09.09.2015
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

#ifndef DBA_GENES_HPP
#define DBA_GENES_HPP

#include <string>

#include <mongo/bson/bson.h>

#include "../datatypes/metadata.hpp"

#include "../parser/fpkm.hpp"
#include "../parser/gtf.hpp"

namespace epidb {
  namespace dba {
    namespace genes {

      bool build_metadata(const std::string &name, const std::string &norm_name,
                          const std::string &description, const std::string &norm_description,
                          const std::string &format,
                          datatypes::Metadata extra_metadata,
                          const std::string &user_key, const std::string &ip,
                          std::string &geneset_id,
                          mongo::BSONObj &geneset_metadata,
                          std::string &msg);


      bool insert(const std::string &name, const std::string &norm_name,
                  const std::string &description, const std::string &norm_description,
                  datatypes::Metadata extra_metadata,
                  const parser::GTFPtr &gtf,
                  const std::string &user_key, const std::string &ip,
                  std::string &gene_model_id, std::string &msg);

      bool insert_expression(const std::string& sample_id, const int replica, datatypes::Metadata extra_metadata,
                             const parser::FPKMPtr &fpkm,  const std::string &user_key, const std::string &ip,
                             std::string &gene_expression_id, std::string &msg);

      bool get_gene_attribute(const std::string& chromosome, const Position start, const Position end,
                              const std::string& attribute_name, const std::string& gene_model,
                              std::string& attibute_value, std::string& msg);

      bool get_gene(const std::string& chromosome, const Position start, const Position end, const std::string& gene_model,
                    mongo::BSONObj& gene, std::string& msg);

      bool get_genes(const std::vector<std::string> &chromosomes, const Position start, const Position end, const std::vector<std::string>& genes_names_or_id,
                     const std::string &user_key, const std::vector<std::string> &norm_gene_models,  std::vector<mongo::BSONObj>& genes, std::string &msg);


      bool get_genes_from_database(const std::vector<std::string> &chromosomes, const Position start, const Position end,
                                   const std::vector<std::string>& genes, const std::string& norm_gene_model,
                                   ChromosomeRegionsList& chromosomeRegionsList, std::string& msg );

      bool get_gene_expressions_from_database(const std::vector<std::string> &sample_ids, const  std::vector<int>& replicates,
                                              const std::vector<std::string>& chromosomes, const int start, const  int end, const std::string& gene_model,  ChromosomeRegionsList& chromosomeRegionsList, std::string& msg);

    }
  }
}

#endif /* defined(DBA_GENES_HPP) */