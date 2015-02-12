//
//  set_biosource_parent.cpp
//  epidb
//
//  Created by Felipe Albrecht on 30.07.13.
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

    class SetBioSourceParentCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::BIOSOURCE_RELATIONSHIP, "Sets a biosource parent.");
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


        std::string norm_bigger_scope = utils::normalize_name(bigger_scope);
        std::string norm_smaller_scope = utils::normalize_name(smaller_scope);

        bool bigger_scope_is_biosource = dba::exists::biosource(norm_bigger_scope);
        bool bigger_scope_is_syn = dba::exists::biosource_synonym(norm_bigger_scope);
        bool smaller_scope_is_biosource = dba::exists::biosource(norm_smaller_scope);
        bool smaller_scope_is_syn = dba::exists::biosource_synonym(norm_smaller_scope);

        if (!(bigger_scope_is_biosource || bigger_scope_is_syn)) {
          std::string s = Error::m(ERR_INVALID_BIOSOURCE_NAME, bigger_scope.c_str());
          EPIDB_LOG_TRACE(s);
          result.add_error(s);
          return false;
        }

        if (!(smaller_scope_is_biosource || smaller_scope_is_syn)) {
          std::string s = Error::m(ERR_INVALID_BIOSOURCE_NAME, smaller_scope.c_str());
          EPIDB_LOG_TRACE(s);
          result.add_error(s);
          return false;
        }

        std::string msg;
        if (dba::set_biosource_parent(bigger_scope, norm_bigger_scope,
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
