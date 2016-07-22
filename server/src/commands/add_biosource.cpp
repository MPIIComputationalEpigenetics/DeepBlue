//
//  add_biosource.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 16.06.13.
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

#include "../dba/controlled_vocabulary.hpp"
#include "../dba/dba.hpp"
#include "../dba/exists.hpp"
#include "../dba/list.hpp"
#include "../datatypes/user.hpp"
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
        return CommandDescription(categories::BIOSOURCES, "Add a BioSource to DeepBlue. A BioSource refers to a term describing the origin of a given sample, such as a tissue or cell line.");
      }

      static Parameters parameters_()
      {
        Parameter p[] = {
          Parameter("name", serialize::STRING, "biosource name"),
          Parameter("description", serialize::STRING, "description of the biosource"),
          parameters::AdditionalExtraMetadata,
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
        datatypes::User user;

        if (!check_permissions(user_key, datatypes::INCLUDE_COLLECTION_TERMS, user, msg )) {
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
