//
//  set_bio_source_synonym.cpp
//  epidb
//
//  Created by Felipe Albrecht on 01.08.13.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include "../extras/utils.hpp"
#include "../extras/serialize.hpp"
#include "../dba/dba.hpp"

#include "../engine/commands.hpp"

namespace epidb {
    namespace command {

      class SetBioSourceSynonymCommand: public Command {

      private:
        static CommandDescription desc_() {
          return CommandDescription(categories::BIO_SOURCE_RELATIONSHIP, "Sets a bio source synonym.");
        }

        static  Parameters parameters_() {
          Parameter p[] = {
            Parameter("bio_source", serialize::STRING, "name of the bio source"),
            Parameter("synonym_name", serialize::STRING, "name of the synonym"),
            parameters::UserKey
          };
          Parameters params(&p[0], &p[0]+3);
          return params;
        }

        static Parameters results_() {
          Parameter p[] = {};
          Parameters results(&p[0], &p[0]+0);
          return results;
        }

      public:
        SetBioSourceSynonymCommand() : Command("set_bio_source_synonym", parameters_(), results_(), desc_()) {}

        virtual bool run(const std::string& ip,
            const serialize::Parameters& parameters, serialize::Parameters& result) const
        {
          const std::string bio_source_name = parameters[0]->as_string();
          const std::string synonym_name = parameters[1]->as_string();
          const std::string user_key = parameters[2]->as_string();

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

          bool is_bio_source(false);
          bool is_syn(false);
          std::string norm_bio_source_name = utils::normalize_name(bio_source_name);
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
            ss << ". No bio_source or synonymous was defined with this name.";
            result.add_error(ss.str());
            return false;
          }

          std::string norm_synoynm_name = utils::normalize_name(synonym_name);

          bool exists(false);
          if (!dba::check_bio_source(norm_synoynm_name, exists, msg)) {
            result.add_error(msg);
            return false;
          }
          if (exists) {
            std::stringstream ss;
            ss << "Invalid synonymous name: ";
            ss << bio_source_name ;
            ss << ". A synonymous with this name already exists.";
            result.add_error(ss.str());
            return false;
          }

          if (!dba::check_bio_source_synonym(norm_synoynm_name, exists, msg)) {
            result.add_error(msg);
            return false;
          }
          if (exists) {
            std::stringstream ss;
            ss << "Invalid synonymous name: ";
            ss << bio_source_name ;
            ss << ". A bio_source synonym with this name already exists.";
            result.add_error(ss.str());
            return false;
          }

          if (!dba::set_bio_source_synonym(bio_source_name, synonym_name, is_bio_source, is_syn, user_key, msg)) {
            result.add_error(msg);
            return false;
          }

          return true;
        }

      } setBioSourceSynonymCommand;
  }
}
