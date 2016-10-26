//
//  expressions.hpp
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


#include <algorithm>
#include <string>
#include <vector>

#include <mongo/bson/bson.h>

#include "../dba/collections.hpp"
#include "../dba/helpers.hpp"
#include "../dba/info.hpp"
#include "../dba/key_mapper.hpp"
#include "../dba/users.hpp"

#include "../parser/parser_factory.hpp"

#include "../macros.hpp"

#include "expressions.hpp"

namespace epidb {
  namespace datatypes {

  	std::vector<ExpressionTypePtr> ExpressionManager::__registered;

    bool ExpressionManager::is_expression_type(const std::string& name)
    {
      return std::find_if(__registered.begin(), __registered.end(),
      [&name](ExpressionTypePtr const & et) {
        return et->name() == name;
      }) != __registered.end();
    }

    const std::vector<ExpressionTypePtr>& ExpressionManager::registered_expression_types()
    {
      return __registered;
    }

    bool AbstractExpressionType::build_expression_metadata(const std::string &sample_id, const int replica,
        const std::string &format,
        const std::string &project,
        const std::string &norm_project,
        const mongo::BSONObj& extra_metadata_obj,
        const std::string &user_key, const std::string &ip,
        int &dataset_id,
        std::string &gene_model_id,
        mongo::BSONObj &gene_model_metadata,
        std::string &msg)
    {

    	NEW_DATASET_ID(dataset_id, msg)
    	BUILD_ID(GENE_EXPRESSIONS, gx, gene_model_id, msg)

      int _id;
      if (!dba::helpers::get_increment_counter(dba::Collections::GENE_EXPRESSIONS(), _id, msg) ||
          !dba::helpers::notify_change_occurred(dba::Collections::GENE_EXPRESSIONS(), msg))  {
        return false;
      }
      gene_model_id = "gx" + utils::integer_to_string(_id);

      mongo::BSONObjBuilder gene_model_metadata_builder;
      gene_model_metadata_builder.append("_id", gene_model_id);
      gene_model_metadata_builder.append(dba::KeyMapper::DATASET(), dataset_id);
      gene_model_metadata_builder.append("sample_id", sample_id);
      gene_model_metadata_builder.append("replica", replica);
      gene_model_metadata_builder.append("extra_metadata", extra_metadata_obj);

      if (format == "cufflinks") {
        const auto& cufflinks_format = parser::FileFormat::cufflinks_format();
        gene_model_metadata_builder.append("format", cufflinks_format.format());
        gene_model_metadata_builder.append("columns", cufflinks_format.to_bson());
      } else if (format == "grape2") {
        const auto& grape2_format = parser::FileFormat::grape2_format();
        gene_model_metadata_builder.append("format", grape2_format.format());
        gene_model_metadata_builder.append("columns", grape2_format.to_bson());
      } else {
        msg = "Format '" + format + "' is unknow";
        return false;
      }

      gene_model_metadata_builder.append("project", project);
      gene_model_metadata_builder.append("norm_project", norm_project);

      datatypes::Metadata sample_data;
      if (!dba::info::get_sample_by_id(sample_id, sample_data, msg, true)) {
        return false;
      }
      mongo::BSONObjBuilder sample_builder;
      for (auto it = sample_data.begin(); it != sample_data.end(); ++it) {
        if ((it->first != "_id") && (it->first != "user")) {
          sample_builder.append(it->first, it->second);
        }
      }
      gene_model_metadata_builder.append("sample_info", sample_builder.obj());

      gene_model_metadata = gene_model_metadata_builder.obj();
      return true;
    }

    bool AbstractExpressionType::build_upload_info(const std::string &user_key, const std::string &client_address, const std::string &content_format,
        mongo::BSONObj &upload_info, std::string &msg)
    {
      utils::IdName user;
      if (!dba::users::get_user(user_key, user, msg)) {
        return false;
      }

      mongo::BSONObjBuilder upload_info_builder;

      upload_info_builder.append("user", user.id);
      upload_info_builder.append("content_format", content_format);
      upload_info_builder.append("done", false);
      upload_info_builder.append("client_address", client_address);
      time_t time_;
      time(&time_);
      upload_info_builder.appendTimeT("upload_start", time_);

      upload_info = upload_info_builder.obj();

      return true;
    }

  }
}
