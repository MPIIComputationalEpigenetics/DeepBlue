//
//  add_gene_set.cpp
//  epidb
//
//  Created by Felipe Albrecht on 08.07.2015
//  Copyright (c) 2015 Max Planck Institute for Computer Science. All rights reserved.
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

    class AddGeneSetCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::GENES, "Inserts a new set of genes in the GTF format.");
      }

      static Parameters parameters_()
      {
        Parameter p[] = {
          Parameter("name", serialize::STRING, "gene set name"),
          Parameter("description", serialize::STRING, "description of the annotation"),
          Parameter("data", serialize::DATASTRING, "the BED formatted data"),
          Parameter("format", serialize::STRING, "Currently, it is only supported GTF."),
          Parameter("extra_metadata", serialize::MAP, "additional metadata"),
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 6);
        return params;
      }

      static Parameters results_()
      {
        Parameter p[] = {
          Parameter("id", serialize::STRING, "id of the newly inserted annotation")
        };
        Parameters results(&p[0], &p[0] + 1);
        return results;
      }

    public:
      AddGeneSetCommand() : Command("add_gene_set", parameters_(), results_(), desc_()) {}

      // TODO: Check user
      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        const std::string name = parameters[0]->as_string();
        const std::string description = parameters[1]->as_string();
        const std::string data = parameters[2]->as_string();
        const std::string format = parameters[3]->as_string();
        const std::string user_key = parameters[5]->as_string();

        std::string msg;
        datatypes::User user;

        if (!check_permissions(user_key, datatypes::INCLUDE_ANNOTATIONS, user, msg )) {
          result.add_error(msg);
          return false;
        }

        datatypes::Metadata extra_metadata;
        if (!read_metadata(parameters[4], extra_metadata, msg)) {
          result.add_error(msg);
          return false;
        }

        std::string norm_name = utils::normalize_name(name);
        std::string norm_description = utils::normalize_name(description);

        if (dba::exists::gene_set(norm_name)) {
          std::string s = "The gene set name " + name + " is already being used.";
          result.add_error(s);
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
        bool ret = dba::gene_set::insert(name, norm_name, description, norm_description,
                                         extra_metadata, gtf, user_key, ip, id, msg);

        if (ret) {
          result.add_string(id);
        } else {
          result.add_error(msg);
        }

        return true;
      }

    } addGeneSetCommand;
  }
}
