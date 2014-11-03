//
//  get_biosource_related.cpp
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
        return CommandDescription(categories::BIOSOURCE_RELATIONSHIP, "Gets biosources related to the given one.");
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
      GetBioSourceRelatedCommand() : Command("get_biosource_related", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        const std::string biosource_name = parameters[0]->as_string();
        const std::string user_key = parameters[1]->as_string();

        std::string norm_biosource_name = utils::normalize_name(biosource_name);

        bool is_biosource(false);
        bool is_syn(false);

        std::string msg;
        if (!Command::checks(user_key, msg)) {
          result.add_error(msg);
          return false;
        }

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

        std::vector<utils::IdName> related_biosources;

        if (!dba::get_biosource_scope(biosource_name, norm_biosource_name,
                                       is_biosource, user_key, related_biosources, msg)) {
          result.add_error(msg);
          return false;
        }


        std::vector<utils::IdName> final_result;
        BOOST_FOREACH(const utils::IdName& related_biosource, related_biosources) {
          std::vector<utils::IdName> related_syns;
          std::string norm_related_biosource = utils::normalize_name(related_biosource.name);

          if (!dba::check_biosource(norm_related_biosource, is_biosource, msg)) {
            result.add_error(msg);
            return false;
          }

          if (!dba::get_biosource_synonyms(related_biosource.name, norm_related_biosource, is_biosource, user_key, related_syns, msg)) {
            result.add_error(msg);
            return false;
          }

          BOOST_FOREACH(const utils::IdName& related_syn, related_syns) {
            final_result.push_back(related_syn);
          }
        }

        set_id_names_return(final_result, result);

        return true;
      }

    } getBioSourceRelatedCommand;
  }
}
