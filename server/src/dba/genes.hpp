//
//  genes.cpp
//  epidb
//
//  Created by Felipe Albrecht on 09.09.2015
//  Copyright (c) 2016 Max Planck Institute for Informatics. All rights reserved.
//

#ifndef DBA_GENES_HPP
#define DBA_GENES_HPP

#include <string>

#include <mongo/bson/bson.h>

#include "../datatypes/metadata.hpp"

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
                  std::string &gene_set_id, std::string &msg);

      bool get_genes_from_database(const std::vector<std::string>& genes, const std::string& gene_set,
                                   ChromosomeRegionsList& chromosomeRegionsList, std::string& msg );

    }
  }
}

#endif /* defined(DBA_GENES_HPP) */