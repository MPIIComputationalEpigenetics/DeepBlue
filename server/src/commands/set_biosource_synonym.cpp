//
//  set_biosource_synonym.cpp
//  epidb
//
//  Created by Felipe Albrecht on 01.08.13.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include "../dba/dba.hpp"
#include "../dba/exists.hpp"

#include "../extras/utils.hpp"
#include "../extras/serialize.hpp"

#include "../engine/commands.hpp"

#include "../log.hpp"
#include "../errors.hpp"

namespace epidb {
  namespace command {

    class SetBioSourceSynonymCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::BIOSOURCE_RELATIONSHIP, "Sets a biosource synonym.");
      }

      static  Parameters parameters_()
      {
        Parameter p[] = {
          Parameter("biosource", serialize::STRING, "name of the biosource"),
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
        if (!Command::checks(user_key, msg)) {
          result.add_error(msg);
          return false;
        }

        const std::string norm_biosource_name = utils::normalize_name(biosource_name);

        // TODO Move to a helper function: get_biosource_root
        // Check if the actual biosource exists
        bool is_biosource = dba::exists::biosource(norm_biosource_name);
        bool is_syn = dba::exists::biosource_synonym(norm_biosource_name);

        if (!(is_biosource || is_syn)) {
          std::string s = Error::m(ERR_INVALID_BIOSOURCE_NAME, biosource_name.c_str());
          EPIDB_LOG_TRACE(s);
          result.add_error(s);
          return false;
        }

        // TODO Move to a helper function: get_biosource_root
        // Check if synonym name is already being user
        std::string norm_synoynm_name = utils::normalize_name(synonym_name);
        bool syn_is_biosource = dba::exists::biosource(norm_synoynm_name);
        bool syn_is_syn = dba::exists::biosource_synonym(norm_synoynm_name);

        if (syn_is_biosource || syn_is_syn) {
          std::string s = Error::m(ERR_INVALID_BIOSOURCE_SYNONYM, synonym_name.c_str());
          EPIDB_LOG_TRACE(s);
          result.add_error(s);
          return false;
        }

        if (!dba::set_biosource_synonym(biosource_name, synonym_name, is_biosource, is_syn, user_key, msg)) {
          result.add_error(msg);
          return false;
        }

        result.add_string(synonym_name);

        return true;
      }

    } setBioSourceSynonymCommand;
  }
}
