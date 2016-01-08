//
//  get_biosource_children.cpp
//  epidb
//
//  Created by Felipe Albrecht on 20.08.13.
//  Copyright (c) 2016 Max Planck Institute for Informatics. All rights reserved.
//

#include "../dba/dba.hpp"
#include "../dba/exists.hpp"
#include "../datatypes/user.hpp"
#include "../extras/utils.hpp"
#include "../extras/serialize.hpp"

#include "../engine/commands.hpp"

#include "../log.hpp"
#include "../errors.hpp"

namespace epidb {
  namespace command {

    class GetBioSourceChildrenCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::BIOSOURCE_RELATIONSHIP, "Gets the scope for the biosource.");
      }

      static  Parameters parameters_()
      {
        Parameter p[] = {
          Parameter("biosource", serialize::STRING, "name of the biosource"),
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 2);
        return params;
      }

      static Parameters results_()
      {
        Parameter p[] = {
          Parameter("biosources", serialize::LIST, "related biosources")
        };
        Parameters results(&p[0], &p[0] + 1);
        return results;
      }

    public:
      GetBioSourceChildrenCommand() : Command("get_biosource_children", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        const std::string biosource_name = parameters[0]->as_string();
        const std::string user_key = parameters[1]->as_string();

        std::string norm_biosource_name = utils::normalize_name(biosource_name);

        std::string msg;
        datatypes::User user;

        if (!check_permissions(user_key, datatypes::GET_DATA, user, msg )) {
          result.add_error(msg);
          return false;
        }

        bool is_biosource = dba::exists::biosource(norm_biosource_name);
        bool is_syn = dba::exists::biosource_synonym(norm_biosource_name);

        if (!(is_biosource || is_syn)) {
          std::string s = Error::m(ERR_INVALID_BIOSOURCE_NAME, biosource_name);
          EPIDB_LOG_TRACE(s);
          result.add_error(s);
          return false;
        }

        std::vector<utils::IdName> related_biosources;

        if (!dba::get_biosource_children(biosource_name, norm_biosource_name,
                                         is_biosource, user_key, related_biosources, msg)) {
          result.add_error(msg);
          return false;
        }

        set_id_names_return(related_biosources, result);

        return true;
      }

    } getBioSourceChildrenCommand;
  }
}
