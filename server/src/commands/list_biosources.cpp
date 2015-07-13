//
//  list_biosources.cpp
//  epidb
//
//  Created by Felipe Albrecht on 17.06.13.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include "../engine/commands.hpp"

#include "../dba/dba.hpp"
#include "../dba/list.hpp"
#include "../datatypes/user.hpp"
#include "../entities/users.hpp"
#include "../extras/utils.hpp"
#include "../extras/serialize.hpp"
#include "../errors.hpp"

namespace epidb {
  namespace command {

    class ListBioSourcesCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::BIOSOURCES, "Lists all existing biosources.");
      }

      static Parameters parameters_()
      {
        Parameter p[] = {
          Parameter("metadata", serialize::MAP, "metadadata to be matched"),
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 2);
        return params;
      }

      static Parameters results_()
      {
        Parameter p[] = {
          Parameter("biosources", serialize::LIST, "biosources")
        };
        Parameters results(&p[0], &p[0] + 1);
        return results;
      }

    public:
      ListBioSourcesCommand() : Command("list_biosources", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {

        const std::string user_key = parameters[1]->as_string();

        std::string msg;
        
        datatypes::User user;
        if (!dba::get_user_by_key(user_key, user, msg)) {
          result.add_error(msg);
          return false;
        }
        
        if (!user.has_permission(datatypes::LIST_COLLECTIONS)) {
            result.add_error(Error::m(ERR_INSUFFICIENT_PERMISSION));
            return false;
        }

        datatypes::Metadata metadata;
        if (!read_metadata(parameters[0], metadata, msg)) {
          result.add_error(msg);
          return false;
        }

        std::vector<utils::IdName> names;
        if (!dba::list::biosources(metadata, names, msg)) {
          result.add_error(msg);
          return false;
        }

        set_id_names_return(names, result);

        return true;
      }
    } listBioSourcesCommand;
  }
}

