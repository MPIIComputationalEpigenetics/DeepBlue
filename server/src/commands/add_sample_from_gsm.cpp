//
//  add_sample_from_gsm.cpp
//  epidb
//
//  Created by Felipe Albrecht on 10.12.14.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include "../engine/commands.hpp"

#include "../dba/controlled_vocabulary.hpp"
#include "../dba/data.hpp"
#include "../dba/dba.hpp"
#include "../dba/exists.hpp"
#include "../dba/list.hpp"
#include "../datatypes/user.hpp"
#include "../external/geo.hpp"

#include "../extras/utils.hpp"
#include "../extras/serialize.hpp"

#include "../errors.hpp"
#include "../log.hpp"

namespace epidb {
  namespace command {

    class AddSampleFromGSMCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::SAMPLES, "Import sample from an existing GSM identifier.");
      }

      static Parameters parameters_()
      {
        Parameter p[] = {
          Parameter("name", serialize::STRING, "biosource name"),
          Parameter("gsm_id", serialize::STRING, "GSM ID"),
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
      AddSampleFromGSMCommand() : Command("add_sample_from_gsm", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        const std::string biosource_name = parameters[0]->as_string();
        const std::string gsm_id = parameters[1]->as_string();
        const std::string user_key = parameters[2]->as_string();

        const std::string norm_biosource_name = utils::normalize_name(biosource_name);

        std::string msg;
        datatypes::User user;

        if (!check_permissions(user_key, datatypes::INCLUDE_COLLECTION_TERMS, user, msg )) {
          result.add_error(msg);
          return false;
        }

        mongo::BSONArray s_array;
        std::vector<std::string> existing_ids;
        datatypes::Metadata verify_metadata;
        verify_metadata["GSM_SAMPLE"] = gsm_id;
        if (!dba::list::samples(user_key, s_array, verify_metadata, existing_ids, msg)) {
          result.add_error(msg);
          return false;
        }


        // TODO Move to a helper function: get_biosource_root
        bool is_biosource = dba::exists::biosource(norm_biosource_name);
        bool is_syn = dba::exists::biosource_synonym(norm_biosource_name);

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

        if (!existing_ids.empty()) {
          mongo::BSONObj existing_sample;
          if (!dba::data::sample(existing_ids[0], existing_sample, msg)) {
            result.add_error(msg);
            return false;
          }
          std::string existing_sample_biosource = existing_sample["norm_biosource_name"].str();
          if (norm_biosource_name == existing_sample_biosource) {
            result.add_string(existing_ids[0]);
          } else {
            result.add_error("GSM ID '" + gsm_id + "' was already imported with the BioSource '" + biosource_name +"'");
            return false;
          }
          return true;
        }

        datatypes::Metadata metadata;
        if (!external::geo::load_gsm(gsm_id, metadata, msg)) {
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
    } addSampleFromGSMCommand;
  }
}
