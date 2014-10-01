//
//  set_biosource_synonym.cpp
//  epidb
//
//  Created by Felipe Albrecht on 01.08.13.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include "../extras/utils.hpp"
#include "../extras/serialize.hpp"
#include "../dba/dba.hpp"

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
        bool ok;
        if (!dba::check_user(user_key, ok, msg)) {
          result.add_error(msg);
          return false;
        }
        if (!ok) {
          std::string s = Error::m(ERR_INVALID_USER_KEY);
          result.add_error(s);
          return false;
        }

        bool is_biosource(false);
        bool is_syn(false);
        std::string norm_biosource_name = utils::normalize_name(biosource_name);
        if (!dba::check_biosource(norm_biosource_name, is_biosource, msg)) {
          result.add_error(msg);
          return false;
        }

        if (!is_biosource) {
          if (!dba::check_biosource_synonym(norm_biosource_name, is_syn, msg)) {
            result.add_error(msg);
            return false;
          }
        }

        if (!(is_biosource || is_syn)) {
          std::string s = Error::m(ERR_INVALID_BIOSOURCE_NAME, biosource_name.c_str());
          EPIDB_LOG_TRACE(s);
          result.add_error(s);
          return false;
        }

        std::string norm_synoynm_name = utils::normalize_name(synonym_name);

        bool exists(false);
        if (!dba::check_biosource(norm_synoynm_name, exists, msg)) {
          result.add_error(msg);
          return false;
        }
        if (exists) {
          std::string s = Error::m(ERR_INVALID_BIOSOURCE_SYNONYM, synonym_name.c_str());
          EPIDB_LOG_TRACE(s);
          result.add_error(s);
          return false;
        }

        if (!dba::check_biosource_synonym(norm_synoynm_name, exists, msg)) {
          result.add_error(msg);
          return false;
        }
        if (exists) {
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
