//
//  add_sample.cpp
//  epidb
//
//  Created by Felipe Albrecht on 30.09.13.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include "../engine/commands.hpp"

#include "../dba/controlled_vocabulary.hpp"
#include "../dba/dba.hpp"
#include "../dba/list.hpp"

#include "../extras/utils.hpp"
#include "../extras/serialize.hpp"

#include "../errors.hpp"
#include "../log.hpp"

namespace epidb {
  namespace command {

    class AddSampleCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::SAMPLES, "Inserts a new sample with the given parameters.");
      }

      static Parameters parameters_()
      {
        Parameter p[] = {
          Parameter("name", serialize::STRING, "sample name"),
          Parameter("metadata", serialize::MAP, "sample metadata"),
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 3);
        return params;
      }

      static Parameters results_()
      {
        Parameter p[] = {
          Parameter("id", serialize::STRING, "id of the newly inserted sample")
        };
        Parameters results(&p[0], &p[0] + 1);
        return results;
      }

    public:
      AddSampleCommand() : Command("add_sample", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        const std::string biosource_name = parameters[0]->as_string();
        const std::string user_key = parameters[2]->as_string();

        std::string msg;
        bool ok = false;
        if (!dba::check_user(user_key, ok, msg)) {
          result.add_error(msg);
          return false;
        }
        if (!ok) {
          std::string s = Error::m(ERR_INVALID_USER_KEY);
          result.add_error(s);
          return false;
        }

        const std::string norm_biosource_name = utils::normalize_name(biosource_name);

        bool is_biosource(false);
        bool is_syn(false);
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

        std::string sample_biosource_name;
        std::string norm_sample_biosource_name;
        if (is_syn) {
          if (!dba::cv::get_synonym_root(biosource_name, norm_biosource_name,
                                         sample_biosource_name, norm_sample_biosource_name, msg)) {
            return false;
          }
        } else {
          sample_biosource_name = biosource_name;
          norm_sample_biosource_name = norm_biosource_name;
        }

        datatypes::Metadata metadata;
        if (!read_metadata(parameters[1], metadata, msg)) {
          result.add_error(msg);
          return false;
        }

        std::string s_id;
        if (!dba::add_sample(sample_biosource_name, norm_sample_biosource_name, metadata, user_key, s_id, msg)) {
          result.add_error(msg);
          return false;
        }

        result.add_string(s_id);

        return true;
      }
    } addSampleCommand;
  }
}
