//
//  info.cpp
//  epidb
//
//  Created by Fabian Reinartz on 01.10.2013.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include <map>

#include <boost/foreach.hpp>

#include "../dba/dba.hpp"
#include "../dba/info.hpp"
#include "../extras/serialize.hpp"

#include "../engine/commands.hpp"

namespace epidb {
  namespace command {

    class InfoCommand : public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::GENERAL_INFORMATION,
                                  "Gets the information for the given id which includes all its public attributes.");
      }

      static  Parameters parameters_()
      {
        Parameter p[] = {
          Parameter("id", serialize::STRING, "id of the data", true),
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 2);
        return params;
      }

      static Parameters results_()
      {
        Parameter p[] = {
          Parameter("information", serialize::MAP, "information about the object")
        };
        Parameters results(&p[0], &p[0] + 1);
        return results;
      }

    public:
      InfoCommand() : Command("info", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        const std::string user_key = parameters[1]->as_string();

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

        std::vector<serialize::ParameterPtr> ids_param;
        parameters[0]->children(ids_param);

        if (ids_param.size() > 1) {
          result.set_as_array(true);
        }

        BOOST_FOREACH(serialize::ParameterPtr id_param, ids_param) {
          std::string id = id_param->as_string();
          std::string type;
          std::map<std::string, std::string> res;
          std::map<std::string, std::string> metadata;
          if (id.compare(0, 1, "a") == 0) {
            ok = dba::info::get_annotation(id, res, metadata, msg);
            type = "annotation";
          } else if (id.compare(0, 1, "g") == 0) {
            ok = dba::info::get_genome(id, res, msg);
            type = "genome";
          } else if (id.compare(0, 1, "p") == 0) {
            ok = dba::info::get_project(id, res, msg);
            type = "project";
          } else if (id.compare(0, 2, "bs") == 0) {
            ok = dba::info::get_bio_source(id, res, metadata, msg);
            type = "bio_source";
          } else if (id.compare(0, 1, "s") == 0) {
            ok = dba::info::get_sample_by_id(id, res, msg);
            type = "sample";
          } else if (id.compare(0, 2, "em") == 0) {
            ok = dba::info::get_epigenetic_mark(id, res, msg);
            type = "epigenetic_mark";
          } else if (id.compare(0, 1, "e") == 0) {
            ok = dba::info::get_experiment(id, res, metadata, msg);
            type = "experiment";
          } else if (id.compare(0, 1, "q") == 0) {
            ok = dba::info::get_query(id, res, msg);
            type = "query";
          } else if (id.compare(0, 2, "tr") == 0) {
            ok = dba::info::get_tiling_region(id, res, msg);
            type = "tiling_region";
          } else if (id.compare(0, 1, "t") == 0) {
            ok = dba::info::get_technique(id, res, metadata, msg);
            type = "technique";
          } else if (id.compare(0, 1, "f") == 0) {
            ok = dba::info::get_sample_field(id, res, msg);
            type = "sample_field";
          } else {
            result.add_error("Invalid identifier: " + id);
            return false;
          }
          if (!ok) {
            result.add_error(msg);
            return false;
          }

          serialize::ParameterPtr info(new serialize::MapParameter());

          serialize::ParameterPtr pt(new serialize::SimpleParameter(serialize::STRING, type));
          info->add_child("type", pt);

          std::map<std::string, std::string>::iterator it;
          for (it = res.begin(); it != res.end(); ++it) {
            serialize::ParameterPtr p(new serialize::SimpleParameter(serialize::STRING, it->second));
            info->add_child(it->first, p);
          }

          if (metadata.size() > 0) {
            serialize::ParameterPtr extra_metadata(new serialize::MapParameter());
            std::map<std::string, std::string>::iterator it;
            for (it = metadata.begin(); it != metadata.end(); ++it) {
              serialize::ParameterPtr p(new serialize::SimpleParameter(serialize::STRING, it->second));
              extra_metadata->add_child(it->first, p);
            }
            info->add_child("extra_metadata", extra_metadata);
          }
          result.add_param(info);
        }

        return true;
      }

    } infoCommand;
  }
}
