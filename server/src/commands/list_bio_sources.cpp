//
//  list_bio_sources.cpp
//  epidb
//
//  Created by Felipe Albrecht on 17.06.13.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include "../engine/commands.hpp"

#include "../dba/dba.hpp"
#include "../dba/list.hpp"

#include "../extras/utils.hpp"
#include "../extras/serialize.hpp"

namespace epidb {
    namespace command {

      class ListBioSourcesCommand: public Command {

      private:
        static CommandDescription desc_() {
          return CommandDescription(categories::BIO_SOURCES, "Lists all existing bio sources.");
        }

        static Parameters parameters_() {
          Parameter p[] = {
            parameters::UserKey
          };
          Parameters params(&p[0], &p[0]+1);
          return params;
        }

        static Parameters results_() {
          Parameter p[] = {
            Parameter("bio_sources", serialize::LIST, "bio sources")
          };
          Parameters results(&p[0], &p[0]+1);
          return results;
        }

      public:
        ListBioSourcesCommand() : Command("list_bio_sources", parameters_(), results_(), desc_()) {}

        virtual bool run(const std::string& ip,
            const serialize::Parameters& parameters, serialize::Parameters& result) const
        {
          const std::string user_key = parameters[0]->as_string();

          std::string msg;
          bool ok;
          if (!dba::check_user(user_key, ok, msg)) {
            result.add_error(msg);
            return false;
          }
          if (!ok) {
            result.add_error("Invalid user key.");
            return false;
          }

          std::vector<utils::IdName> names;
          if (!dba::list::bio_sources(user_key, names, msg)) {
            result.add_error(msg);
            return false;
          }

          set_id_names_return(names, result);

          return true;
        }
      } listBioSourcesCommand;
  }
}

