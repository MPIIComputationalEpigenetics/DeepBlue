//
//  list_in_use.cpp
//  epidb
//
//  Created by Felipe Albrecht on 12.10.14.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include <boost/foreach.hpp>

#include "../engine/commands.hpp"

#include "../dba/collections.hpp"
#include "../dba/dba.hpp"
#include "../dba/list.hpp"

#include "../extras/utils.hpp"
#include "../extras/serialize.hpp"

#include "../errors.hpp"

namespace epidb {
  namespace command {

    class ListInUseCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::GENERAL_INFORMATION, "Lists all terms from the given controlled vocabulary that are used.");
      }

      static Parameters parameters_()
      {
        Parameter p[] = {
          Parameter("controlled_vocabulary", serialize::STRING, "id of the data"),
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 2);
        return params;
      }

      static Parameters results_()
      {
        Parameter p[] = {
          Parameter("terms", serialize::LIST, "controlled_vocabulary terms with count")
        };
        Parameters results(&p[0], &p[0] + 1);
        return results;
      }

    public:
      ListInUseCommand() : Command("list_in_use", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        const std::string controlled_vocabulary = parameters[0]->as_string();
        const std::string user_key = parameters[1]->as_string();

        std::string msg;
        if (!Command::checks(user_key, msg)) {
          result.add_error(msg);
          return false;
        }

        std::string collection_key;
        if (controlled_vocabulary == dba::Collections::EPIGENETIC_MARKS()) {
          collection_key = "$norm_epigenetic_mark";
        } else if (controlled_vocabulary == dba::Collections::GENOMES()) {
          collection_key = "$norm_genome";
        } else if (controlled_vocabulary == dba::Collections::BIOSOURCES()) {
          collection_key = "$sample_info.norm_biosource_name";
        } else if (controlled_vocabulary == dba::Collections::SAMPLES()) {
          collection_key = "$sample_id";
        } else if (controlled_vocabulary == dba::Collections::TECHNIQUES()) {
          collection_key = "$norm_technique";
        } else if (controlled_vocabulary == dba::Collections::PROJECTS()) {
          collection_key = "$norm_project";
        } else {
          result.add_error("Controlled vocabulary " + controlled_vocabulary + " does not exist. Available controlled vocabularies: " +
                           dba::Collections::EPIGENETIC_MARKS() + ", " + dba::Collections::GENOMES() + ", " +
                           dba::Collections::BIOSOURCES() + ", " + dba::Collections::SAMPLES() + ", " +
                           dba::Collections::TECHNIQUES() + ", " + dba::Collections::PROJECTS());
          return false;
        }

        std::vector<utils::IdNameCount> names;
        if (!dba::list::in_use(controlled_vocabulary, collection_key, user_key, names, msg)) {
          result.add_error(msg);
          return false;
        }

        set_id_names_count_return(names, result);

        return true;
      }
    } listInUseCommand;
  }
}
