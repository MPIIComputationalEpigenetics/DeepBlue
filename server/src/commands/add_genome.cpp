//
//  add_genome.cpp
//  epidb
//
//  Created by Felipe Albrecht on 25.06.13.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include "../dba/dba.hpp"
#include "../extras/utils.hpp"
#include "../extras/serialize.hpp"
#include "../parser/genome_data.hpp"

#include "../engine/commands.hpp"

namespace epidb {
    namespace command {

      class AddGenomeCommand: public Command {

      private:
        static CommandDescription desc_() {
          return CommandDescription(categories::GENOMES, "Inserts a new genome with the given parameters.");
        }

        static Parameters parameters_() {
          Parameter p[] = {
            Parameter("name", serialize::STRING, "genome name"),
            Parameter("description", serialize::STRING, "description of the genome"),
            Parameter("data", serialize::DATASTRING, "genome data"),
            parameters::UserKey
          };
          Parameters params(&p[0], &p[0]+4);
          return params;
        }

        static Parameters results_() {
          Parameter p[] = {
            Parameter("id", serialize::STRING, "id of the newly inserted genome")
          };
          Parameters results(&p[0], &p[0]+1);
          return results;
        }

      public:
        AddGenomeCommand() : Command("add_genome", parameters_(), results_(), desc_()) {}

        virtual bool run(const std::string& ip,
            const serialize::Parameters& parameters, serialize::Parameters& result) const
        {
          const std::string name = parameters[0]->as_string();
          const std::string description = parameters[1]->as_string();
          const std::string data = parameters[2]->as_string();
          const std::string user_key = parameters[3]->as_string();

          std::string msg;
          bool ok;
          if (!dba::check_user(user_key, ok, msg)) {
            result.add_error(msg);
            return false;
          }
          if (!ok) {
            result.add_error("Invalid user key.");
            return false;
          }

          std::string norm_name = utils::normalize_name(name);
          if (!dba::is_valid_genome(name, norm_name, msg)) {
            result.add_error(msg);
            return false;
          }

          parser::ChromosomesInfo gi;
          if (!parser::string_to_genome_info(data, gi, msg)) {
            result.add_error(msg);
            return false;
          }

          if (gi.empty()) {
            result.add_error("It is necessary to define at least one chromosome for the genome");
            return false;
          }

          std::string norm_description = utils::normalize_name(description);

          std::string id;
          bool ret = dba::add_genome(name, norm_name, description, norm_description, gi, user_key, ip, id, msg);

          if (!ret) {
            result.add_error(msg);
          } else {
            result.add_string(id);
          }
          return ret;
        }
      } addGenomeCommand;
  }
}

