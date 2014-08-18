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
        const std::string bio_source_name = parameters[0]->as_string();
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

        const std::string norm_bio_source_name = utils::normalize_name(bio_source_name);

        bool is_bio_source(false);
        bool is_syn(false);
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

        std::string sample_bio_source_name;
        std::string norm_sample_bio_source_name;
        if (is_syn) {
          if (!dba::cv::get_synonym_root(bio_source_name, norm_bio_source_name,
                                         sample_bio_source_name, norm_sample_bio_source_name, msg)) {
            return false;
          }
        } else {
          sample_bio_source_name = bio_source_name;
          norm_sample_bio_source_name = norm_bio_source_name;
        }

        Metadata metadata;
        if (!read_metadata(parameters[1], metadata, msg)) {
          result.add_error(msg);
          return false;
        }

        std::string s_id;
        if (!dba::add_sample(sample_bio_source_name, norm_sample_bio_source_name, metadata, user_key, s_id, msg)) {
          result.add_error(msg);
          return false;
        }

        result.add_string(s_id);

        return true;
      }
    } addSampleCommand;
  }
}
