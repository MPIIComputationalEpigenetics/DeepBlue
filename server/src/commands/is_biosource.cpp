//
//  get_biosource_by_name.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 14.11.2013.
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

#include "../dba/controlled_vocabulary.hpp"
#include "../dba/dba.hpp"
#include "../dba/exists.hpp"

#include "../datatypes/user.hpp"

#include "../extras/serialize.hpp"

#include "../engine/commands.hpp"

#include "../errors.hpp"


namespace epidb {
  namespace command {

    class IsBiosourceCommand : public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::GENERAL_INFORMATION, "Verify if the name is an existing and valid DeepBlue BioSource name. A BioSource refers to a term describing the origin of a given sample, such as a tissue or cell line.");
      }

      static  Parameters parameters_()
      {
        Parameter p[] = {
          parameters::BioSource,
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 2);
        return params;
      }

      static Parameters results_()
      {
        Parameter p[] = {
          Parameter("information", serialize::STRING, "A string containing the biosource name", true)
        };
        Parameters results(&p[0], &p[0] + 1);
        return results;
      }

    public:
      IsBiosourceCommand() : Command("is_biosource", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        const std::string biosource_name = parameters[0]->as_string();
        const std::string user_key = parameters[1]->as_string();

        std::string msg;
        datatypes::User user;

        if (!check_permissions(user_key, datatypes::LIST_COLLECTIONS, user, msg )) {
          result.add_error(msg);
          return false;
        }

        const std::string norm_biosource_name = utils::normalize_name(biosource_name);

        // TODO Move to a helper function: get_biosource_root
        bool is_biosource = dba::exists::biosource(norm_biosource_name);
        bool is_syn = dba::exists::biosource_synonym(norm_biosource_name);

        if (!(is_biosource || is_syn)) {
          std::string s = Error::m(ERR_INVALID_BIOSOURCE_NAME, biosource_name);
          EPIDB_LOG_TRACE(s);
          result.add_error(s);
          return false;
        }

        std::string sample_biosource_name;
        std::string norm_sample_biosource_name;
        if (is_syn) {
          if (!dba::cv::get_synonym_root(biosource_name, norm_biosource_name,
                                         sample_biosource_name, norm_sample_biosource_name, msg)) {
            return false;
          }
        } else {
          sample_biosource_name = biosource_name;
        }

        result.add_string(sample_biosource_name);

        return true;
      }

    } isBiosourceCommand;
  }
}
