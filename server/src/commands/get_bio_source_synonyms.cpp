//
//  set_bio_source_synonym.cpp
//  epidb
//
//  Created by Felipe Albrecht on 01.08.13.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include <boost/foreach.hpp>

#include "../dba/dba.hpp"
#include "../extras/utils.hpp"
#include "../extras/serialize.hpp"

#include "../engine/commands.hpp"

#include "../log.hpp"
#include "../errors.hpp"

namespace epidb {
  namespace command {

    class GetBioSourceSynonymCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::BIO_SOURCE_RELATIONSHIP, "Gets the synonyms for the bio source.");
      }

      static Parameters parameters_()
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
          Parameter("synonyms", serialize::LIST, "synonyms of the bio source")
        };
        Parameters results(&p[0], &p[0] + 1);
        return results;
      }

    public:
      GetBioSourceSynonymCommand() : Command("get_bio_source_synonyms", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        const std::string bio_source_name = parameters[0]->as_string();
        const std::string user_key = parameters[1]->as_string();

        std::string msg;

        {
          // TODO: put in a auxiliar function
          bool ok(false);
          if (!dba::check_user(user_key, ok, msg)) {
            result.add_error(msg);
            return false;
          }
          if (!ok) {
            result.add_error("Invalid user key.");
            return false;
          }
        }

        std::string norm_bio_source_name = utils::normalize_name(bio_source_name);

        bool is_bio_source(false);
        bool is_syn(false);
        if (!dba::check_bio_source(norm_bio_source_name, is_bio_source, msg)) {
          result.add_error(msg);
          return false;
        }

        if (!is_bio_source) {
          if (!dba::check_bio_source(norm_bio_source_name, is_syn, msg)) {
            result.add_error(msg);
            return false;
          }
        }

        if (!(is_bio_source || !is_syn)) {
          std::string s = Error::m(ERR_INVALID_BIO_SOURCE_NAME, bio_source_name.c_str());
          EPIDB_LOG_TRACE(s);
          result.add_error(s);
          return false;
        }

        std::vector<utils::IdName> syns;
        if (!dba::get_bio_source_synonyms(bio_source_name, norm_bio_source_name, is_bio_source, user_key, syns, msg)) {
          result.add_error(msg);
          return false;
        }

        set_id_names_return(syns, result);

        return true;
      }

    } getBioSourceSynonymCommand;
  }
}
