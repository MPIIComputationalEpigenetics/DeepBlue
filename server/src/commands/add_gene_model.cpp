//
//  add_gene_model.cpp
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

#include "../datatypes/user.hpp"

#include "../engine/commands.hpp"

#include "../extras/utils.hpp"
#include "../extras/serialize.hpp"

#include "../parser/gtf_parser.hpp"

#include "../errors.hpp"

namespace epidb {
  namespace command {

    class AddGeneModelCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::GENES, "Include a Gene Model in DeepBlue. The data must be in the GTF format. Important: this command will include only the lines where the column 'feature' is 'genes'.");
      }

      static Parameters parameters_()
      {
        return {
          parameters::GeneModel,
          parameters::Genome,
          Parameter("description", serialize::STRING, "description of the gene model"),
          Parameter("data", serialize::DATASTRING, "data in the GTF format"),
          Parameter("format", serialize::STRING, "data format - currently, only the GTF format is supported."),
          parameters::AdditionalExtraMetadata,
          parameters::UserKey
        };
      }

      static Parameters results_()
      {
        return {
          Parameter("id", serialize::STRING, "id of the newly inserted gene model")
        };
      }

    public:
      AddGeneModelCommand() : Command("add_gene_model", parameters_(), results_(), desc_()) {}

      // TODO: Check user
      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        const std::string name = parameters[0]->as_string();
        const std::string genome = parameters[1]->as_string();
        const std::string description = parameters[2]->as_string();
        const std::string data = parameters[3]->as_string();
        const std::string format = parameters[4]->as_string();
        const std::string user_key = parameters[6]->as_string();

        std::string msg;
        datatypes::User user;

        if (!check_permissions(user_key, datatypes::INCLUDE_ANNOTATIONS, user, msg )) {
          result.add_error(msg);
          return false;
        }

        datatypes::Metadata extra_metadata;
        if (!read_metadata(parameters[5], extra_metadata, msg)) {
          result.add_error(msg);
          return false;
        }

        std::string norm_name = utils::normalize_name(name);
        std::string norm_description = utils::normalize_name(description);
        std::string norm_genome = utils::normalize_name(genome);

        if (dba::exists::gene_model(norm_name)) {
          std::string s = Error::m(ERR_DUPLICATED_GENE_MODEL_NAME, name);
          result.add_error(s);
          return false;
        }

        if (!dba::exists::genome(norm_genome)) {
          result.add_error("Invalid genome '" + genome + "'");
          return false;
        }

        std::string norm_file_format = utils::normalize_name(format);
        if (norm_file_format != "gtf") {
          std::string s = "Currently, only the format GTF is supported.";
        }


        std::unique_ptr<std::istream> _input = std::unique_ptr<std::istream>(new std::stringstream(data));
        parser::GTFParser parser(std::move(_input));
        parser::GTFPtr gtf;
        if (!parser.get( gtf, msg)) {
          result.add_error(msg);
          return false;
        }

        std::string id;
        bool ret = dba::genes::insert(user, name, norm_name, genome, norm_genome,
                                      description, norm_description,
                                      extra_metadata, gtf, ip, id, msg);

        if (ret) {
          result.add_string(id);
        } else {
          result.add_error(msg);
        }

        return ret;
      }

    } addGeneModelCommand;
  }
}
