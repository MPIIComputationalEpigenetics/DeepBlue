//
//  add_gene_expression.cpp
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

#include "../engine/commands.hpp"

#include "../extras/utils.hpp"
#include "../extras/serialize.hpp"

#include "../parser/cufflinks_parser.hpp"
#include "../parser/fpkm.hpp"

#include "../errors.hpp"

namespace epidb {
  namespace command {

    class AddGeneExpressionCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::GENES, "Include a Gene Expression in DeepBlue. The data must be in the XXXX format.");
      }

      static Parameters parameters_()
      {
        Parameter p[] = {
          Parameter("sample_id", serialize::STRING, "sample ID"),
          Parameter("replica", serialize::INTEGER, "replica count (use 0 if it is the single replica)"),
          Parameter("data", serialize::DATASTRING, "the cufflinks formatted data"),
          Parameter("format", serialize::STRING, "Currently, it is only supported cufflinks."),
          Parameter("project", serialize::STRING, "the project name"),
          parameters::AdditionalExtraMetadata,
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 7);
        return params;
      }

      static Parameters results_()
      {
        Parameter p[] = {
          Parameter("id", serialize::STRING, "id of the newly inserted gene expression data")
        };
        Parameters results(&p[0], &p[0] + 1);
        return results;
      }

    public:
      AddGeneExpressionCommand() : Command("add_gene_expression", parameters_(), results_(), desc_()) {}

      // TODO: Check user
      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        const std::string sample_id = parameters[0]->as_string();
        const int replica = parameters[1]->as_long();
        const std::string data = parameters[2]->as_string();
        const std::string format = parameters[3]->as_string();
        const std::string project = parameters[4]->as_string();
        const std::string user_key = parameters[6]->as_string();

        std::string msg;
        datatypes::User user;

        if (!check_permissions(user_key, datatypes::INCLUDE_EXPERIMENTS, user, msg )) {
          result.add_error(msg);
          return false;
        }

        datatypes::Metadata extra_metadata;
        if (!read_metadata(parameters[5], extra_metadata, msg)) {
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

        std::string norm_file_format = utils::normalize_name(format);
        if (norm_file_format != "cufflinks") {
          std::string s = "Currently, only the format 'cufflinks' is supported.";
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

        std::unique_ptr<std::istream> _input = std::unique_ptr<std::istream>(new std::stringstream(data));
        parser::CufflinksParser parser(std::move(_input));
        parser::FPKMPtr fpkm_file;
        if (!parser.get(fpkm_file, msg)) {
          result.add_error(msg);
          return false;
        }

        std::string id;
        bool ret = dba::genes::insert_expression(sample_id, replica, extra_metadata, fpkm_file, project, norm_project, user_key, ip, id, msg);
        if (ret) {
          result.add_string(id);
        } else {
          result.add_error(msg);
        }

        return true;
      }

    } addGeneExpressionCommand;
  }
}
