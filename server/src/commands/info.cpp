//
//  info.cpp
//  epidb
//
//  Created by Fabian Reinartz on 01.10.2013.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include <map>
#include <sstream>
#include <iostream>

#include "../datatypes/user.hpp"

#include "../dba/dba.hpp"
#include "../dba/info.hpp"
#include "../dba/users.hpp"
#include "../dba/list.hpp"

#include "../engine/commands.hpp"
#include "../engine/engine.hpp"
#include "../engine/request.hpp"

#include "../extras/serialize.hpp"
#include "../extras/utils.hpp"

#include "../errors.hpp"

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

      bool get_request(const std::string& id, const std::string& user_key,
                       std::map<std::string, std::string>& map, std::string& msg) const
      {

        datatypes::User user;
        if (!dba::users::get_user_by_key(user_key, user, msg)) {
          return false;
        }

        request::Job job;
        if (user.is_admin() || epidb::Engine::instance().user_owns_request(id, user.get_id())) {
          if (!epidb::Engine::instance().request_job(id, job, msg)) {
            return false;
          }
        } else {
          msg = "Request ID " + id + " not found.";
          return false;
        }

        map["_id"] = job._id;
        map["state"] = job.status.state;
        map["message"] = job.status.message;
        map["command"] = job.command;
        map["user_id"] = job.command;
        std::stringstream ss;
        ss << job.create_time;
        map["create_time"] = ss.str();
        ss.str("");
        ss << job.finish_time;
        if (job.status.state == "done") {
          map["finish_time"] = ss.str();
        }
        map["query_id"] = job.query_id;

        return true;
      }

      bool get_user(const std::string& key, std::map<std::string, std::string>& metadata, std::string& msg) const
      {
        datatypes::User user;
        if (!dba::users::get_user_by_key(key, user, msg)) {
          return false;
        }
        metadata["id"] = user.get_id();
        metadata["name"] = user.get_name();
        metadata["email"] = user.get_email();
        metadata["institution"] = user.get_institution();

        datatypes::PermissionLevel pl = user.get_permission_level();
        std::string pl_string;

        metadata["permission_level"] = datatypes::permission_level_to_string(pl);

        return true;
      }

    public:
      InfoCommand() : Command("info", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        const std::string user_key = parameters[1]->as_string();

        std::string msg;
        datatypes::User user;

        std::string err_msg;
        bool has_list_collections_permissions = false;
        if (check_permissions(user_key, datatypes::LIST_COLLECTIONS, user, msg )) {
          has_list_collections_permissions = true;
        } else {
          err_msg = msg;
        }

        std::vector<utils::IdName> user_projects_id_names;
        if (!dba::list::projects(user_key, user_projects_id_names, msg)) {
          result.add_error(msg);
          return false;
        }

        std::vector<std::string> user_projects;
        for (const auto& project : user_projects_id_names) {
          user_projects.push_back(utils::normalize_name(project.name));
        }

        std::vector<serialize::ParameterPtr> ids_param;
        parameters[0]->children(ids_param);

        if (ids_param.empty()) {
          result.set_as_array(true);
          return true;
        }

        std::vector<std::string> synonyms;
        for (const serialize::ParameterPtr & id_param : ids_param) {
          std::string id = id_param->as_string();
          std::string type;
          std::map<std::string, std::string> metadata;
          std::map<std::string, std::string> extra_metadata;
          std::map<std::string, std::string> sample_info;
          std::map<std::string, std::string> upload_info;
          std::vector<std::map<std::string, std::string> > columns;
          bool ok;

          // Any user can request information about himself
          if (id == "me") {
            ok = get_user(user_key, metadata, msg);
            type = "user";
          } else {
            // must have LIST_COLLECTIONS permission to get information about the other data types.
            if (!has_list_collections_permissions) {
              result.add_error(err_msg);
              return false;
            }

            if (id.compare(0, 1, "a") == 0) {
              ok = dba::info::get_annotation(id, metadata, extra_metadata, columns, upload_info, msg);
              ok = ok && dba::info::id_to_name(upload_info, msg);
              type = "annotation";
            } else if (id.compare(0, 1, "g") == 0) {
              ok = dba::info::get_genome(id, metadata, msg);
              ok = ok && dba::info::id_to_name(metadata, msg);
              type = "genome";
            } else if (id.compare(0, 1, "p") == 0) {
              ok = dba::info::get_project(id, user_projects, metadata, msg);
              ok = ok && dba::info::id_to_name(metadata, msg);
              type = "project";
            } else if (id.compare(0, 2, "bs") == 0) {
              ok = dba::info::get_biosource(id, metadata, extra_metadata, synonyms, msg);
              ok = ok && dba::info::id_to_name(metadata, msg);
              type = "biosource";
            } else if (id.compare(0, 1, "s") == 0) {
              ok = dba::info::get_sample_by_id(id, metadata, msg);
              ok = ok && dba::info::id_to_name(metadata, msg);
              type = "sample";
            } else if (id.compare(0, 2, "em") == 0) {
              ok = dba::info::get_epigenetic_mark(id, metadata, extra_metadata, msg);
              ok = ok && dba::info::id_to_name(metadata, msg);
              type = "epigenetic_mark";
            } else if (id.compare(0, 1, "e") == 0) {
              ok = dba::info::get_experiment(id, user_projects, metadata, extra_metadata, sample_info, columns, upload_info, msg);
              ok = ok && dba::info::id_to_name(upload_info, msg);
              type = "experiment";
            } else if (id.compare(0, 1, "q") == 0) {
              ok = dba::info::get_query(id, metadata, msg);
              if (!user.is_admin() && metadata["user"] != user.get_id()) {
                msg = Error::m(ERR_PERMISSION_QUERY, id);
                ok = false;
              }
              ok = ok && dba::info::id_to_name(metadata, msg);
              type = "query";
            } else if (id.compare(0, 2, "tr") == 0) {
              ok = dba::info::get_tiling_region(id, metadata, msg);
              ok = ok && dba::info::id_to_name(metadata, msg);
              type = "tiling_region";
            } else if (id.compare(0, 1, "t") == 0) {
              ok = dba::info::get_technique(id, metadata, extra_metadata, msg);
              ok = ok && dba::info::id_to_name(metadata, msg);
              type = "technique";
            } else if (id.compare(0, 2, "ct") == 0) {
              ok = dba::info::get_column_type(id, metadata, msg);
              type = "column_type";
            } else if (id.compare(0, 1, "r") == 0) {
              ok = get_request(id, user_key, metadata, msg);
              type = "request";
            } else {
              result.add_error("Invalid identifier: " + id);
              return false;
            }
          }

          if (!ok) {
            result.add_error(msg);
            return false;
          }

          result.set_as_array(true);
          serialize::ParameterPtr info(new serialize::MapParameter());

          serialize::ParameterPtr pt(new serialize::SimpleParameter(serialize::STRING, type));
          info->add_child("type", pt);

          std::map<std::string, std::string>::iterator it;
          for (it = metadata.begin(); it != metadata.end(); ++it) {
            serialize::ParameterPtr p(new serialize::SimpleParameter(serialize::STRING, it->second));
            info->add_child(it->first, p);
          }

          if (!synonyms.empty()) {
            serialize::ParameterPtr serialize_synonyms(new serialize::ListParameter());

            for (const auto& syn : synonyms) {
              serialize::ParameterPtr p(new serialize::SimpleParameter(serialize::STRING, syn));
              serialize_synonyms->add_child(p);
            }

            info->add_child("synonyms", serialize_synonyms);
          }

          if (!extra_metadata.empty()) {
            serialize::ParameterPtr extra_metadata_parameter(new serialize::MapParameter());
            std::map<std::string, std::string>::iterator it;
            for (it = extra_metadata.begin(); it != extra_metadata.end(); ++it) {
              serialize::ParameterPtr p(new serialize::SimpleParameter(it->second));
              extra_metadata_parameter->add_child(it->first, p);
            }
            info->add_child("extra_metadata", extra_metadata_parameter);
          }

          if (!sample_info.empty()) {
            serialize::ParameterPtr sample_info_parameter(new serialize::MapParameter());
            std::map<std::string, std::string>::iterator it;
            for (it = sample_info.begin(); it != sample_info.end(); ++it) {
              serialize::ParameterPtr p(new serialize::SimpleParameter(it->second));
              sample_info_parameter->add_child(it->first, p);
            }
            info->add_child("sample_info", sample_info_parameter);
          }

          if (!upload_info.empty()) {
            serialize::ParameterPtr upload_info_parameter(new serialize::MapParameter());
            std::map<std::string, std::string>::iterator it;
            for (it = upload_info.begin(); it != upload_info.end(); ++it) {
              serialize::ParameterPtr p(new serialize::SimpleParameter(it->second));
              upload_info_parameter->add_child(it->first, p);
            }
            info->add_child("upload_info", upload_info_parameter);
          }

          if (!columns.empty()) {
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
