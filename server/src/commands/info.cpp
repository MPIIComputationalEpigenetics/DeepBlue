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
                                  "Return information for the given ID (or IDs).");
      }

      static  Parameters parameters_()
      {
        Parameter p[] = {
          Parameter("id", serialize::STRING, "ID or an array of IDs", true),
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 2);
        return params;
      }

      static Parameters results_()
      {
        Parameter p[] = {
          Parameter("information", serialize::LIST, "List of Maps, where each map contains the info of an object.", true)
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

        bool ok;
        std::string msg;
        if (!Command::checks(user_key, msg)) {
          result.add_error(msg);
          return false;
        }

        std::vector<serialize::ParameterPtr> ids_param;
        parameters[0]->children(ids_param);

        result.set_as_array(true);

        std::vector<std::string> synonyms;
        BOOST_FOREACH(const serialize::ParameterPtr & id_param, ids_param) {
          std::string id = id_param->as_string();
          std::string type;
          std::map<std::string, std::string> metadata;
          std::map<std::string, std::string> extra_metadata;
          std::map<std::string, std::string> sample_info;
          std::map<std::string, std::string> upload_info;
          std::vector<std::map<std::string, std::string> > columns;
          if (id.compare(0, 1, "a") == 0) {
            ok = dba::info::get_annotation(id, metadata, extra_metadata, columns, upload_info, msg);
            type = "annotation";
          } else if (id.compare(0, 1, "g") == 0) {
            ok = dba::info::get_genome(id, metadata, msg);
            type = "genome";
          } else if (id.compare(0, 1, "p") == 0) {
            ok = dba::info::get_project(id, metadata, msg);
            type = "project";
          } else if (id.compare(0, 2, "bs") == 0) {
            ok = dba::info::get_biosource(id, metadata, extra_metadata, synonyms, msg);
            type = "biosource";
          } else if (id.compare(0, 1, "s") == 0) {
            ok = dba::info::get_sample_by_id(id, metadata, msg);
            type = "sample";
          } else if (id.compare(0, 2, "em") == 0) {
            ok = dba::info::get_epigenetic_mark(id, metadata, msg);
            type = "epigenetic_mark";
          } else if (id.compare(0, 1, "e") == 0) {
            ok = dba::info::get_experiment(id, metadata, extra_metadata, sample_info, columns, upload_info, msg);
            type = "experiment";
          } else if (id.compare(0, 1, "q") == 0) {
            ok = dba::info::get_query(id, metadata, msg);
            type = "query";
          } else if (id.compare(0, 2, "tr") == 0) {
            ok = dba::info::get_tiling_region(id, metadata, msg);
            type = "tiling_region";
          } else if (id.compare(0, 1, "t") == 0) {
            ok = dba::info::get_technique(id, metadata, extra_metadata, msg);
            type = "technique";
          } else if (id.compare(0, 2, "ct") == 0) {
            ok = dba::info::get_column_type(id, metadata, msg);
            type = "column_type";
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
          for (it = metadata.begin(); it != metadata.end(); ++it) {
            serialize::ParameterPtr p(new serialize::SimpleParameter(serialize::STRING, it->second));
            info->add_child(it->first, p);
          }

          if (synonyms.size() > 0) {
            serialize::ParameterPtr serialize_synonyms(new serialize::ListParameter());
            std::vector<std::string>::iterator it_syns = synonyms.begin();

            for ( ; it_syns != synonyms.end(); it_syns++) {
              serialize::ParameterPtr p(new serialize::SimpleParameter(serialize::STRING, *it_syns));
              serialize_synonyms->add_child(p);
            }

            info->add_child("synonyms", serialize_synonyms);
          }

          if (extra_metadata.size() > 0) {
            serialize::ParameterPtr extra_metadata_parameter(new serialize::MapParameter());
            std::map<std::string, std::string>::iterator it;
            for (it = extra_metadata.begin(); it != extra_metadata.end(); ++it) {
              serialize::ParameterPtr p(new serialize::SimpleParameter(it->second));
              extra_metadata_parameter->add_child(it->first, p);
            }
            info->add_child("extra_metadata", extra_metadata_parameter);
          }

          if (sample_info.size() > 0) {
            serialize::ParameterPtr sample_info_parameter(new serialize::MapParameter());
            std::map<std::string, std::string>::iterator it;
            for (it = sample_info.begin(); it != sample_info.end(); ++it) {
              serialize::ParameterPtr p(new serialize::SimpleParameter(it->second));
              sample_info_parameter->add_child(it->first, p);
            }
            info->add_child("sample_info", sample_info_parameter);
          }

          if (upload_info.size() > 0) {
            serialize::ParameterPtr upload_info_parameter(new serialize::MapParameter());
            std::map<std::string, std::string>::iterator it;
            for (it = upload_info.begin(); it != upload_info.end(); ++it) {
              serialize::ParameterPtr p(new serialize::SimpleParameter(it->second));
              upload_info_parameter->add_child(it->first, p);
            }
            info->add_child("upload_info", upload_info_parameter);
          }

          if (columns.size() > 0) {
            serialize::ParameterPtr columns_parameters(new serialize::ListParameter());
            for (std::vector< std::map<std::string, std::string> >::iterator c_it = columns.begin(); c_it != columns.end(); c_it++) {

              serialize::ParameterPtr column_parameter(new serialize::MapParameter());
              std::map<std::string, std::string>::iterator it;
              for (it = c_it->begin(); it != c_it->end(); ++it) {
                serialize::ParameterPtr p(new serialize::SimpleParameter(serialize::STRING, it->second));
                column_parameter->add_child(it->first, p);
              }
              columns_parameters->add_child(column_parameter);
            }
            info->add_child("columns", columns_parameters);
          }

          result.add_param(info);
        }

        return true;
      }

    } infoCommand;
  }
}
