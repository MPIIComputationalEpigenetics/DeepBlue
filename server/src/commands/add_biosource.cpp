//
//  add_biosource.cpp
//  epidb
//
//  Created by Felipe Albrecht on 16.06.13.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include "../dba/controlled_vocabulary.hpp"
#include "../dba/dba.hpp"
#include "../dba/exists.hpp"
#include "../dba/list.hpp"
#include "../extras/utils.hpp"
#include "../extras/serialize.hpp"

#include "../engine/commands.hpp"

#include "../errors.hpp"

namespace epidb {
  namespace command {

    class AddBioSourceCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::BIOSOURCES, "Inserts a new biosource with the given parameters.");
      }

      static Parameters parameters_()
      {
        Parameter p[] = {
          Parameter("name", serialize::STRING, "biosource name"),
          Parameter("description", serialize::STRING, "description of the biosource"),
          Parameter("extra_metadata", serialize::MAP, "additional metadata"),
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 4);
        return params;
      }

      static Parameters results_()
      {
        Parameter p[] = {
          Parameter("id", serialize::STRING, "id of the newly inserted biosource")
        };
        Parameters results(&p[0], &p[0] + 1);
        return results;
      }

    public:
      AddBioSourceCommand() : Command("add_biosource", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        const std::string name = parameters[0]->as_string();
        const std::string description = parameters[1]->as_string();

        const std::string user_key = parameters[3]->as_string();

        std::string msg;
        if (!Command::checks(user_key, msg)) {
          result.add_error(msg);
          return false;
        }

        datatypes::Metadata extra_metadata;
        if (!read_metadata(parameters[2], extra_metadata, msg)) {
          result.add_error(msg);
          return false;
        }

        //// Check if a biosource with the same ontology_id already exists.
        //// If it does exists, include the new biosource as a synonym
        auto ontology_id = extra_metadata.find("ontology_id");
        if (ontology_id != extra_metadata.end()) {
          datatypes::Metadata search_metadata;
          search_metadata["ontology_id"] = ontology_id->second;
          std::vector<utils::IdName> names;
          if (!dba::list::biosources(search_metadata, names, msg)) {
            result.add_error(msg);
            return false;
          }
          if (!names.empty()) {
            if (names.size() > 1) {
              result.add_error("Fatal error, it was expected only one BioSource with the ontology_id:" + ontology_id->second + ". But more than one was found. Please, contact us. Meanwhile, you can use the equivalent biosource \"" + names[0].name + "\".");
              return false;
            }
            bool ret = dba::cv::set_biosource_synonym_complete(names[0].name, name, user_key, msg);
            if (!ret) {
              result.add_error(msg);
            } else {
              result.add_string(names[0].id);
            }
            return ret;
          }
        }
        ////

        // Normal add_biosource code flow.
        std::string norm_name = utils::normalize_name(name);
        if (!dba::is_valid_biosource_name(name, norm_name, msg)) {
          result.add_error(msg);
          return false;
        }

        std::string norm_description = utils::normalize_name(description);

        std::string id;
        bool ret = dba::add_biosource(name, norm_name, description, norm_description, extra_metadata, user_key, id, msg);
        if (!ret) {
          result.add_error(msg);
        } else {
          result.add_string(id);
        }
        return ret;
      }
    } addBioSourceCommand;
  }
}
