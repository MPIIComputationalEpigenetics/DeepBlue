//
//  set_biosource_synonym.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 01.08.13.
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

#include "../datatypes/user.hpp"

#include "../engine/commands.hpp"

#include "../extras/serialize.hpp"

#include "../log.hpp"
#include "../errors.hpp"

namespace epidb {
  namespace command {

    class SetBioSourceSynonymCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::BIOSOURCE_RELATIONSHIP, "Define a synonym for a BioSource. BioSources can have multiple synonyms, just execute this command for each synonyms.");
      }

      static  Parameters parameters_()
      {
        Parameter p[] = {
          parameters::BioSource,
          Parameter("synonym_name", serialize::STRING, "name of the synonym"),
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 3);
        return params;
      }

      static Parameters results_()
      {
        Parameter p[] = {
          Parameter("synonym_name", serialize::STRING, "inserted synonym_name")
        };
        Parameters results(&p[0], &p[0] + 1);
        return results;
      }

    public:
      SetBioSourceSynonymCommand() : Command("set_biosource_synonym", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        const std::string biosource_name = parameters[0]->as_string();
        const std::string synonym_name = parameters[1]->as_string();
        const std::string user_key = parameters[2]->as_string();

        std::string msg;
        datatypes::User user;

        if (!check_permissions(user_key, datatypes::INCLUDE_COLLECTION_TERMS, user, msg )) {
          result.add_error(msg);
          return false;
        }

        if (!dba::cv::set_biosource_synonym_complete(biosource_name, synonym_name, user_key, msg)) {
          result.add_error(msg);
          return false;
        }

        result.add_string(synonym_name);

        return true;
      }

    } setBioSourceSynonymCommand;
  }
}
