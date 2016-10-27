//
//  add_expression.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 08.07.2015
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

#include <string>
#include <sstream>

#include "../dba/dba.hpp"
#include "../dba/exists.hpp"
#include "../dba/genes.hpp"
#include "../dba/insert.hpp"
#include "../dba/list.hpp"

#include "../datatypes/user.hpp"
#include "../datatypes/expressions_manager.hpp"

#include "../engine/commands.hpp"

#include "../extras/utils.hpp"
#include "../extras/serialize.hpp"

#include "../parser/gene_expression_parser_factory.hpp"

#include "../interfaces/serializable.hpp"

#include "../errors.hpp"
#include "../macros.hpp"

namespace epidb {
  namespace command {

    class AddExpressionCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::EXPRESSIONS, "Include Expression data in DeepBlue.");
      }

      static Parameters parameters_()
      {
        return {
          parameters::ExpressionType,
          Parameter("sample_id", serialize::STRING, "sample ID"),
          Parameter("replica", serialize::INTEGER, "replica count (use 0 if it is the single replica)"),
          Parameter("data", serialize::DATASTRING, "the data in the right format"),
          Parameter("format", serialize::STRING, "cufflinks or grape2"),
          Parameter("project", serialize::STRING, "the project name"),
          parameters::AdditionalExtraMetadata,
          parameters::UserKey
        };
      }

      static Parameters results_()
      {
        return {
          Parameter("id", serialize::STRING, "id of the newly inserted expression data")
        };
      }

    public:
      AddExpressionCommand() : Command("add_expression", parameters_(), results_(), desc_()) {}

      // TODO: Check user
      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        const std::string expression_type_name = parameters[0]->as_string();
        const std::string sample_id = parameters[1]->as_string();
        const int replica = parameters[2]->as_long();
        const std::string data = parameters[3]->as_string();
        const std::string format = parameters[4]->as_string();
        const std::string project = parameters[5]->as_string();
        const std::string user_key = parameters[7]->as_string();

        std::string msg;
        datatypes::User user;

        if (!check_permissions(user_key, datatypes::INCLUDE_EXPERIMENTS, user, msg )) {
          result.add_error(msg);
          return false;
        }

        datatypes::Metadata extra_metadata;
        if (!read_metadata(parameters[6], extra_metadata, msg)) {
          result.add_error(msg);
          return false;
        }

        std::string norm_sample_id = utils::normalize_name(sample_id);
        std::string norm_project = utils::normalize_name(project);

        if (!dba::exists::sample(norm_sample_id)) {
          std::string s = Error::m(ERR_INVALID_SAMPLE_ID, sample_id);
          result.add_error(s);
          return false;
        }

        GET_EXPRESSION_TYPE(expression_type_name, expression_type)

        if (expression_type->exists(norm_sample_id, replica)) {
          std::string s = Error::m(ERR_DUPLICATE_EXPRESSION, sample_id, replica);
          result.add_error(s);
          return false;
        }

        if (!dba::exists::project(norm_project)) {
          std::vector<utils::IdName> names;
          if (!dba::list::similar_projects(project, user_key, names, msg)) {
            result.add_error(msg);
            return false;
          }
          std::stringstream ss;
          ss << "Invalid project name. ";
          ss << project;
          ss << ".";
          if (!names.empty()) {
            ss << " The following names are suggested: ";
            ss << utils::vector_to_string(names);
          }
          result.add_error(ss.str());
          return false;
        }

        std::string norm_file_format = utils::normalize_name(format);
        if (norm_file_format != "cufflinks"  || norm_file_format == "grape2") {
          msg  = "Currently, only the formats 'cufflinks' or 'grape2' is supported.";
          result.add_error(msg);
          return false;
        }

        auto parser = parser::GeneExpressionParserFactory::build(format, std::unique_ptr<std::istream>(new std::stringstream(data)), msg);

        if (parser == nullptr) {
          result.add_error(msg);
          return false;
        }

        ISerializableFilePtr serializable_file;
        if (!parser->parse(serializable_file, msg)) {
          result.add_error(msg);
          return false;
        }

        std::string id;
        bool ret = expression_type->insert(sample_id, replica, extra_metadata, serializable_file, format, project, norm_project, user_key, ip, id, msg);
        if (ret) {
          result.add_string(id);
        } else {
          result.add_error(msg);
        }
        return ret;
      }

    } addGeneExpressionCommand;
  }
}
