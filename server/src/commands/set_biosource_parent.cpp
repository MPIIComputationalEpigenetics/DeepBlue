//
//  set_biosource_parent.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 30.07.13.
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
#include "../dba/exists.hpp"
#include "../dba/users.hpp"
#include "../datatypes/user.hpp"
#include "../extras/utils.hpp"
#include "../extras/serialize.hpp"

#include "../engine/commands.hpp"

#include "../log.hpp"
#include "../errors.hpp"

namespace epidb {
  namespace command {

    class SetBioSourceParentCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::BIOSOURCE_RELATIONSHIP, "Define a BioSource as parent of another BioSource. This command is used to build the BioSources hierarchy. A BioSource refers to a term describing the origin of a given sample, such as a tissue or cell line.");
      }

      static  Parameters parameters_()
      {
        Parameter p[] = {
          Parameter("parent", serialize::STRING, "parent"),
          Parameter("child", serialize::STRING, "child"),
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 3);
        return params;
      }

      static Parameters results_()
      {
        Parameter p[] = {};
        Parameters results(&p[0], &p[0] + 0);
        return results;
      }

    public:
      SetBioSourceParentCommand() : Command("set_biosource_parent", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        const std::string bigger_scope = parameters[0]->as_string();
        const std::string smaller_scope = parameters[1]->as_string();
        const std::string user_key = parameters[2]->as_string();

        std::string msg;
        datatypes::User user;

        if (!check_permissions(user_key, datatypes::INCLUDE_COLLECTION_TERMS, user, msg )) {
          result.add_error(msg);
          return false;
        }

        std::string norm_bigger_scope = utils::normalize_name(bigger_scope);
        std::string norm_smaller_scope = utils::normalize_name(smaller_scope);

        bool bigger_scope_is_biosource = dba::exists::biosource(norm_bigger_scope);
        bool bigger_scope_is_syn = dba::exists::biosource_synonym(norm_bigger_scope);
        bool smaller_scope_is_biosource = dba::exists::biosource(norm_smaller_scope);
        bool smaller_scope_is_syn = dba::exists::biosource_synonym(norm_smaller_scope);

        if (!(bigger_scope_is_biosource || bigger_scope_is_syn)) {
          std::string s = Error::m(ERR_INVALID_BIOSOURCE_NAME, bigger_scope);
          EPIDB_LOG_TRACE(s);
          result.add_error(s);
          return false;
        }

        if (!(smaller_scope_is_biosource || smaller_scope_is_syn)) {
          std::string s = Error::m(ERR_INVALID_BIOSOURCE_NAME, smaller_scope);
          EPIDB_LOG_TRACE(s);
          result.add_error(s);
          return false;
        }

        if (dba::cv::set_biosource_parent(bigger_scope, norm_bigger_scope,
                                          smaller_scope, norm_smaller_scope,
                                          bigger_scope_is_syn, smaller_scope_is_syn,
                                          user_key, msg)) {
          result.add_string(bigger_scope);
          result.add_string(smaller_scope);
          return true;
        } else {
          result.add_error(msg);
          return false;
        }

      }

    } setBioSourceParentCommand;
  }
}
