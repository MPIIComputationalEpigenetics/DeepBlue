//
//  add_sample.cpp
//  epidb
//
//  Created by Felipe Albrecht on 30.09.13.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include "../engine/commands.hpp"

#include "../dba/dba.hpp"
#include "../dba/list.hpp"

#include "../extras/utils.hpp"
#include "../extras/serialize.hpp"

namespace epidb {
    namespace command {

      class AddSampleCommand: public Command {

      private:
        static CommandDescription desc_() {
          return CommandDescription(categories::SAMPLES, "Inserts a new sample with the given parameters.");
        }

        static Parameters parameters_() {
          Parameter p[] = {
            Parameter("name", serialize::STRING, "sample name"),
            Parameter("metadata", serialize::MAP, "sample metadata"),
            parameters::UserKey
          };
          Parameters params(&p[0], &p[0]+3);
          return params;
        }

        static Parameters results_() {
          Parameter p[] = {
            Parameter("id", serialize::STRING, "id of the newly inserted sample")
          };
          Parameters results(&p[0], &p[0]+1);
          return results;
        }

      public:
        AddSampleCommand() : Command("add_sample", parameters_(), results_(), desc_()) {}

        virtual bool run(const std::string& ip,
            const serialize::Parameters& parameters, serialize::Parameters& result) const
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
            result.add_error("Invalid user key.");
            return false;
          }

          const std::string norm_bio_source_name = utils::normalize_name(bio_source_name);
          if (!dba::check_bio_source(norm_bio_source_name, ok, msg)) {
            result.add_error(msg);
            return false;
          }

          if (!ok) {
            std::vector<utils::IdName> names;
            if (!dba::list::similar_bio_sources(bio_source_name, user_key, names, msg)) {
              result.add_error(msg);
              return false;
            }
            std::stringstream ss;
            ss << "Invalid biological source name '";
            ss << bio_source_name;
            ss << "'. ";
            if (names.size() > 0) {
              ss << "It is suggested the following names: ";
              ss << utils::vector_to_string(names);
            }
            result.add_error(ss.str());
            return false;
          }

          Metadata metadata;
          if (!read_metadata(parameters[1], metadata, msg)) {
            result.add_error(msg);
            return false;
          }

          std::string s_id;
          if (!dba::add_sample(bio_source_name, norm_bio_source_name, metadata, user_key, s_id, msg)) {
            result.add_error(msg);
            return false;
          }

          result.add_string(s_id);

          return true;
        }
      } addSampleCommand;
    }
}
