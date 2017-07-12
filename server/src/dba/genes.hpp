//
//  genes.hpp
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
#include "../datatypes/regions.hpp"
#include "../datatypes/user.hpp"

#include "../parser/fpkm.hpp"
#include "../parser/gtf.hpp"

namespace epidb {
  namespace dba {
    namespace genes {

      void invalidate_cache();

      bool gene_model_info(const std::string& id, mongo::BSONObj& obj_metadata, std::string& msg);

      bool gene_info(const std::string& id, mongo::BSONObj& obj_metadata, std::string& msg);

      bool build_metadata(const std::string &name, const std::string &norm_name,
                          const std::string &description, const std::string &norm_description,
                          const std::string &format,
                          datatypes::Metadata extra_metadata,
                          const std::string &ip,
                          std::string &geneset_id,
                          mongo::BSONObj &geneset_metadata,
                          std::string &msg);


      bool insert(const datatypes::User& user,
                  const std::string &name, const std::string &norm_name,
                  const std::string &genome, const std::string &norm_genome,
                  const std::string &description, const std::string &norm_description,
                  datatypes::Metadata extra_metadata,
                  const parser::GTFPtr &gtf,
                  const std::string &ip,
                  std::string &gene_model_id, std::string &msg);

      bool get_gene_attribute(const std::string& chromosome, const Position start, const Position end, const std::string& strand,
                              const std::string& attribute_name, const std::string& gene_model,
                              std::string& attibute_value, std::string& msg);

      bool get_gene_gene_ontology_annotations(const std::string& chromosome, const Position start, const Position end, const std::string& strand,
                                              const std::string& gene_model,
                                              std::vector<datatypes::GeneOntologyTermPtr>& go_annotations, std::string& msg);

      bool get_gene_by_location(const std::string& chromosome, const Position start, const Position end, const std::string& strand,
                                const std::string& gene_model, RegionPtr& gene, std::string& msg);

      bool get_genes(const std::vector<std::string> &chromosomes, const Position start, const Position end,
                     const std::string& strand,
                     const std::vector<std::string>& genes_names_or_id, const std::vector<std::string>& go_terms,
                     const std::string &norm_gene_model,
                     std::vector<mongo::BSONObj>& genes, std::string &msg);

      bool get_gene_model_obj(const std::string& norm_gene_model,
                              mongo::BSONObj& gene_model_obj, std::string& msg);

      bool get_gene_model_by_dataset_id(const int id, std::string& name, std::string& msg);

      bool get_genes_from_database(const std::vector<std::string> &chromosomes, const Position start, const Position end, const std::string& strand,
                                   const std::vector<std::string>& genes, const std::vector<std::string>& go_terms,
                                   const std::string& norm_gene_model, ChromosomeRegionsList& chromosomeRegionsList, std::string& msg );

      bool map_gene_location(const std::string& gene_tracking_id, const std::string& gene_name, const std::string& gene_model,
                             std::string& chromosome, Position& start, Position& end, std::string& strand, std::string& msg);

      bool exists_gene_ensg(const std::string& gene_ensg_id);

      bool build_genes_database_query(const std::vector<std::string> &chromosomes, const int start, const int end, const std::string& strand,
                                      const std::vector<std::string>& genes, const std::vector<std::string>& go_terms,
                                      const std::string& norm_gene_model,
                                      const bool exactly,
                                      mongo::BSONObj& filter, std::string& msg);

    }
  }
}

#endif /* defined(DBA_GENES_HPP) */