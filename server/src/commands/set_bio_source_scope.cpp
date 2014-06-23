//
//  set_bio_source_scope.cpp
//  epidb
//
//  Created by Felipe Albrecht on 30.07.13.
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

    class SetBioSourceScopeCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::BIO_SOURCE_RELATIONSHIP, "Sets a bio source scope.");
      }

      static  Parameters parameters_()
      {
        Parameter p[] = {
          Parameter("smaller_scope", serialize::STRING, "bigger scope"),
          Parameter("bigger_scope", serialize::STRING, "smaller scope"),
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
      SetBioSourceScopeCommand() : Command("set_bio_source_scope", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        const std::string bigger_scope = parameters[0]->as_string();
        const std::string smaller_scope = parameters[1]->as_string();
        const std::string user_key = parameters[2]->as_string();


        std::string norm_bigger_scope = utils::normalize_name(bigger_scope);
        std::string norm_smaller_scope = utils::normalize_name(smaller_scope);

        bool bigger_scope_is_bio_source(false);
        bool bigger_scope_is_syn(false);
        bool smaller_scope_is_bio_source(false);
        bool smaller_scope_is_syn(false);

        std::string msg;

        bool is_initialized(false);
        if (!dba::is_initialized(is_initialized, msg)) {
          result.add_error(msg);
          return false;
        }
        if (!is_initialized) {
          result.add_error("The system was not initialized.");
          return false;
        }

        bool ok(false);
        if (!dba::check_user(user_key, ok, msg)) {
          result.add_error(msg);
          return false;
        }
        if (!ok) {
          result.add_error("Invalid user key.");
          return false;
        }

        if (!dba::check_bio_source(norm_bigger_scope, bigger_scope_is_bio_source, msg)) {
          result.add_error(msg);
          return false;
        }

        if (!bigger_scope_is_bio_source) {
          if (!dba::check_bio_source_synonym(norm_bigger_scope, bigger_scope_is_syn, msg)) {
            result.add_error(msg);
            return false;
          }
        }

        if (!(bigger_scope_is_bio_source || bigger_scope_is_syn)) {
          std::string s = Error::m(ERR_INVALID_BIO_SOURCE_NAME, bigger_scope.c_str());
          EPIDB_LOG_TRACE(s);
          result.add_error(s);
          return false;
        }

        if (!dba::check_bio_source(norm_smaller_scope, smaller_scope_is_bio_source, msg)) {
          result.add_error(msg);
          return false;
        }

        if (!smaller_scope_is_bio_source) {
          if (!dba::check_bio_source_synonym(norm_smaller_scope, smaller_scope_is_syn, msg)) {
            result.add_error(msg);
            return false;
          }
        }

        if (!(smaller_scope_is_bio_source || smaller_scope_is_syn)) {
          std::string s = Error::m(ERR_INVALID_BIO_SOURCE_NAME, smaller_scope.c_str());
          EPIDB_LOG_TRACE(s);
          result.add_error(s);
          return false;
        }

        if (dba::set_bio_source_scope(bigger_scope, norm_bigger_scope,
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

    } setBioSourceScopeCommand;
  }
}
