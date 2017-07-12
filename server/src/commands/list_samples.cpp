//
//  list_samples.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 30.09.13.
//  Copyright (c) 2016 Max Planck Institute for Informatics. All rights reserved.

//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.

//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.

//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#include <string>
#include <vector>

#include "../engine/commands.hpp"

#include "../dba/controlled_vocabulary.hpp"
#include "../dba/dba.hpp"
#include "../dba/exists.hpp"
#include "../dba/helpers.hpp"
#include "../dba/list.hpp"
#include "../dba/info.hpp"

#include "../datatypes/user.hpp"

#include "../extras/utils.hpp"
#include "../extras/serialize.hpp"

#include "../errors.hpp"

namespace epidb {
  namespace command {

    class ListSamplesCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::SAMPLES, "List Samples included in DeepBlue. It is possible to filter by the BioSource and by extra_metadata fields content.");
      }

      static Parameters parameters_()
      {
        Parameter p[] = {
          parameters::BioSourceMultiple,
          parameters::ExtraMetadata,
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
        datatypes::User user;

        if (!check_permissions(user_key, datatypes::LIST_COLLECTIONS, user, msg )) {
          result.add_error(msg);
          return false;
        }

        std::vector<serialize::ParameterPtr> s;
        parameters[0]->children(s);

        mongo::BSONArrayBuilder ab;
        for (serialize::ParameterPtr &p : s) {
          std::string biosource = p->as_string();
          std::string norm_biosource_name = utils::normalize_name(biosource);

          // TODO Move to a helper function: get_biosource_root
          bool is_biosource = dba::exists::biosource(norm_biosource_name);
          bool is_syn = dba::exists::biosource_synonym(norm_biosource_name);

          if (!(is_biosource || is_syn)) {
            std::string s = Error::m(ERR_INVALID_BIOSOURCE_NAME, biosource);
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

        std::vector<std::string> ids;
        if (!dba::list::samples(s_array, metadata, ids, msg)) {
          result.add_error(msg);
          return false;
        }

        result.set_as_array(true);

        for(const std::string & sample_id: ids) {
          std::map<std::string, std::string> sample_data;
          if (!dba::info::get_sample_by_id(sample_id, sample_data, msg)) {
            return false;
          }

          if (!dba::info::id_to_name(sample_data, msg)) {
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
