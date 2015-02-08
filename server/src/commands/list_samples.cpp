//
//  list_samples.cpp
//  epidb
//
//  Created by Felipe Albrecht on 30.09.13.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include <string>
#include <vector>

#include <boost/foreach.hpp>

#include "../engine/commands.hpp"

#include "../dba/controlled_vocabulary.hpp"
#include "../dba/dba.hpp"
#include "../dba/helpers.hpp"
#include "../dba/list.hpp"
#include "../dba/info.hpp"

#include "../extras/utils.hpp"
#include "../extras/serialize.hpp"

#include "../errors.hpp"

namespace epidb {
  namespace command {

    class ListSamplesCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::SAMPLES, "Lists all existing samples that matches the given biosource and metadata.");
      }

      static Parameters parameters_()
      {
        Parameter p[] = {
          Parameter("biosource", serialize::STRING, "biosource name", true),
          Parameter("metadata", serialize::MAP, "metadadata to be matched"),
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 3);
        return params;
      }

      static Parameters results_()
      {
        Parameter p[] = {
          Parameter("samples", serialize::LIST, "samples id with their content")
        };
        Parameters results(&p[0], &p[0] + 1);
        return results;
      }

    public:
      ListSamplesCommand() : Command("list_samples", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        const std::string user_key = parameters[2]->as_string();

        std::string msg;
        if (!Command::checks(user_key, msg)) {
          result.add_error(msg);
          return false;
        }

        std::vector<serialize::ParameterPtr> s;
        parameters[0]->children(s);

        mongo::BSONArrayBuilder ab;
        BOOST_FOREACH(serialize::ParameterPtr & p, s) {
          std::string biosource = p->as_string();
          std::string norm_biosource_name = utils::normalize_name(biosource);

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
            std::string s = Error::m(ERR_INVALID_BIOSOURCE_NAME, biosource.c_str());
            EPIDB_LOG_TRACE(s);
            result.add_error(s);
            return false;
          }

          std::string biosource_root;
          std::string norm_biosource_root;
          if (is_syn) {
            if (!dba::cv::get_synonym_root(biosource, norm_biosource_name,
                                           biosource_root, norm_biosource_root, msg)) {
              return false;
            }
          } else {
            norm_biosource_root = norm_biosource_name;
          }

          ab.append(norm_biosource_root);
        }

        mongo::BSONArray s_array = ab.arr();


        datatypes::Metadata metadata;
        if (!read_metadata(parameters[1], metadata, msg)) {
          result.add_error(msg);
          return false;
        }

        if (s.empty() && metadata.empty()) {
          result.add_error("At least one BioSource or Metadata information should be informed.");
          return false;
        }


        std::vector<std::string> ids;
        if (!dba::list::samples(user_key, s_array, metadata, ids, msg)) {
          result.add_error(msg);
          return false;
        }

        result.set_as_array(true);

        BOOST_FOREACH(const std::string & sample_id, ids) {
          std::map<std::string, std::string> sample_data;
          if (!dba::info::get_sample_by_id(sample_id, sample_data, msg)) {
            return false;
          }

          serialize::ParameterPtr info(new serialize::MapParameter);
          for (std::map<std::string, std::string>::iterator it = sample_data.begin(); it != sample_data.end(); ++it) {
            serialize::ParameterPtr p(new serialize::SimpleParameter(serialize::STRING, it->second));
            info->add_child(it->first, p);
          }

          serialize::ParameterPtr sample_info(new serialize::ListParameter);
          std::string _id = sample_data["_id"];
          sample_info->add_child(serialize::ParameterPtr(new serialize::SimpleParameter(serialize::STRING, _id)));
          sample_info->add_child(info);

          result.add_param(sample_info);
        }

        return true;
      }
    } listSamplesCommand;
  }
}

