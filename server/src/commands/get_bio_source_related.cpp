//
//  get_bio_source_related.cpp
//  epidb
//
//  Created by Felipe Albrecht on 20.08.13.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include "../dba/dba.hpp"
#include "../extras/utils.hpp"
#include "../extras/serialize.hpp"

#include "../engine/commands.hpp"

#include "../errors.hpp"
#include "../log.hpp"

namespace epidb {
  namespace command {

    class GetBioSourceRelatedCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::BIO_SOURCE_RELATIONSHIP, "Gets bio sources related to the given one.");
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
      GetBioSourceRelatedCommand() : Command("get_bio_source_related", parameters_(), results_(), desc_()) {}

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
          std::string s = Error::m(ERR_INVALID_BIO_SOURCE_NAME, bio_source_name.c_str());
          EPIDB_LOG_TRACE(s);
          result.add_error(s);
          return false;
        }

        std::vector<utils::IdName> related_bio_sources;

        if (!dba::get_bio_source_scope(bio_source_name, norm_bio_source_name,
                                       is_bio_source, user_key, related_bio_sources, msg)) {
          result.add_error(msg);
          return false;
        }


        std::vector<utils::IdName> final_result;
        BOOST_FOREACH(utils::IdName related_bio_source, related_bio_sources) {
          std::vector<utils::IdName> related_syns;
          std::string norm_related_bio_source = utils::normalize_name(related_bio_source.name);

          if (!dba::check_bio_source(norm_related_bio_source, is_bio_source, msg)) {
            result.add_error(msg);
            return false;
          }

          if (!dba::get_bio_source_synonyms(related_bio_source.name, norm_related_bio_source, is_bio_source, user_key, related_syns, msg)) {
            result.add_error(msg);
            return false;
          }

          BOOST_FOREACH(utils::IdName related_syn, related_syns) {
            final_result.push_back(related_syn);
          }
        }

        set_id_names_return(final_result, result);

        return true;
      }

    } getBioSourceRelatedCommand;
  }
}
