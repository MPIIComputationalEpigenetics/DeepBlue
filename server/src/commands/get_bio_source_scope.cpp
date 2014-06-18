//
//  get_bio_source_scope.cpp
//  epidb
//
//  Created by Felipe Albrecht on 20.08.13.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include "../dba/dba.hpp"
#include "../extras/utils.hpp"
#include "../extras/serialize.hpp"

#include "../engine/commands.hpp"

namespace epidb {
  namespace command {

    class GetBioSourceScopeCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::BIO_SOURCE_RELATIONSHIP, "Gets the scope for the bio source.");
      }

      static  Parameters parameters_()
      {
        Parameter p[] = {
          Parameter("bio_source", serialize::STRING, "name of the bio source"),
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 2);
        return params;
      }

      static Parameters results_()
      {
        Parameter p[] = {
          Parameter("bio_sources", serialize::LIST, "related bio sources")
        };
        Parameters results(&p[0], &p[0] + 1);
        return results;
      }

    public:
      GetBioSourceScopeCommand() : Command("get_bio_source_scope", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        const std::string bio_source_name = parameters[0]->as_string();
        const std::string user_key = parameters[1]->as_string();

        std::string norm_bio_source_name = utils::normalize_name(bio_source_name);

        bool is_bio_source(false);
        bool is_syn(false);

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

        if (!dba::check_bio_source(norm_bio_source_name, is_bio_source, msg)) {
          result.add_error(msg);
          return false;
        }

        if (!is_bio_source) {
          if (!dba::check_bio_source_synonym(norm_bio_source_name, is_syn, msg)) {
            result.add_error(msg);
            return false;
          }
        }

        if (!(is_bio_source || is_syn)) {
          std::stringstream ss;
          ss << "Invalid bio_source name: ";
          ss << bio_source_name ;
          ss << ". No Bio Source or synonymous was defined with this name.";
          result.add_error(ss.str());
          return false;
        }

        std::vector<utils::IdName> related_bio_sources;

        if (!dba::get_bio_source_scope(bio_source_name, norm_bio_source_name,
                                       is_bio_source, user_key, related_bio_sources, msg)) {
          return false;
        }

        set_id_names_return(related_bio_sources, result);

        return true;
      }

    } getBioSourceScopeCommand;
  }
}
