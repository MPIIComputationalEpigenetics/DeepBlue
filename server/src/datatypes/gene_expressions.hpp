//
//  gene_expressions.hpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 24.10.16.
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

#ifndef DATATYPES_GENE_EXPRESSIONS_HPP
#define DATATYPES_GENE_EXPRESSIONS_HPP

#include <string>
#include <vector>

#include "expressions.hpp"

#include "../interfaces/serializable.hpp"

namespace epidb {
  namespace datatypes {

    class GeneExpressionType: public AbstractExpressionType {

    public:
      GeneExpressionType() :
        AbstractExpressionType("gene")
      {}

      virtual bool info(const std::string& id, mongo::BSONObj& obj_metadata, std::string& msg);

      virtual bool data(const std::string &id, mongo::BSONObj &result, std::string &msg);

      virtual bool exists(const std::string &sample_id, const int replica);

      virtual bool load_data(const std::vector<std::string> &sample_ids, const  std::vector<long>& replicas,
                             const std::vector<std::string> &genes, const std::vector<std::string> &project,
                             const std::string& norm_gene_model,  ChromosomeRegionsList& chromosomeRegionsList, std::string& msg);

      virtual bool update_upload_info(const std::string &collection, const std::string &annotation_id,
                                      const size_t total_size, const size_t total_genes, std::string &msg);

      virtual mongo::BSONObj to_bson(const int dataset_id, const std::string& gene_id, const ISerializablePtr& row);

      virtual bool insert(const datatypes::User& user,
                          const std::string& sample_id, const int replica, datatypes::Metadata extra_metadata,
                          const ISerializableFilePtr file, const std::string &format,
                          const std::string& project, const std::string& norm_project,
                          const std::string &ip,
                          std::string &expression_id, std::string &msg);

      virtual bool list(const mongo::BSONObj& query, std::vector<utils::IdName> &result, std::string &msg);

    };
  }
}

#endif