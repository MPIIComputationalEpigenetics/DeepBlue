//
//  get_biosource_by_name.cpp
//  epidb
//
//  Created by Felipe Albrecht on 14.11.2013.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
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
        return CommandDescription(categories::GENERAL_INFORMATION,
                                  "Return information for the given biosource name.");
      }

      static  Parameters parameters_()
      {
        Parameter p[] = {
          Parameter("biosource_name", serialize::STRING, "Name of the biosource"),
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
          std::string s = Error::m(ERR_INVALID_BIOSOURCE_NAME, biosource_name.c_str());
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
