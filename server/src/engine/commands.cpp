//
//  commands.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 29.05.13.
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
#include <iostream>

#include <boost/random.hpp>

#include "commands.hpp"

#include "../datatypes/metadata.hpp"
#include "../datatypes/user.hpp"

#include "../dba/dba.hpp"
#include "../dba/users.hpp"

#include "../dba/exists.hpp"

#include "../extras/utils.hpp"

#include "../errors.hpp"
#include "../log.hpp"

namespace epidb {
  std::map<std::string, Command *> *Command::commands_;

  bool operator==(const CommandCategory &c1, const CommandCategory &c2)
  {
    return c1.name == c2.name;
  }

  Command::Command(const std::string &name, const Parameters &params,
                   const Parameters &results, const CommandDescription &desc)
    : name_(name), desc_(desc), parameters_(params), results_(results)
  {
    init();
  }

  void Command::init()
  {
    if ( commands_ == 0 ) {
      commands_ = new std::map<std::string, Command *>;
    }

    const Command *c = (*commands_)[name_];
    if ( c ) {
      EPIDB_LOG_WARN("warning: 2 commands with name: " << name_);
    }

    (*commands_)[name_] = this;
  }

  bool Command::check_permissions(const std::string& user_key, const datatypes::PermissionLevel& permission, datatypes::User &user, std::string& msg ) const
  {
    if (!dba::users::get_user_by_key(user_key, user, msg)) {
      std::cerr << "get user by key fail: " << msg << std::endl;
      return false;
    }

    if (!user.has_permission(permission)) {
      msg = Error::m(ERR_INSUFFICIENT_PERMISSION, datatypes::permission_level_to_string(permission));
      return false;
    }
    return true;
  }

  // TODO: Check how to discover if the given ip is the local ip.
  bool Command::check_local(const std::string &ip, bool &r, std::string &msg) const
  {
    r = true;
    return true;
  }

  void Command::set_id_names_return(const std::vector<utils::IdName> &id_names, serialize::Parameters &result) const
  {
    result.set_as_array(true);
    for (const utils::IdName & id_name : id_names) {
      std::vector<serialize::ParameterPtr> list;
      list.push_back(serialize::ParameterPtr(new serialize::SimpleParameter(serialize::STRING, id_name.id)));
      list.push_back(serialize::ParameterPtr(new serialize::SimpleParameter(serialize::STRING, id_name.name)));
      result.add_list(list);
    }
  }

  void Command::set_id_names_count_return(const std::vector<utils::IdNameCount> &id_name_counts, serialize::Parameters &result) const
  {
    result.set_as_array(true);
    for (const utils::IdNameCount & id_name_count : id_name_counts) {
      std::vector<serialize::ParameterPtr> list;
      list.push_back(serialize::ParameterPtr(new serialize::SimpleParameter(serialize::STRING, id_name_count.id)));
      list.push_back(serialize::ParameterPtr(new serialize::SimpleParameter(serialize::STRING, id_name_count.name)));
      list.push_back(serialize::ParameterPtr(new serialize::SimpleParameter((long long) id_name_count.count)));
      result.add_list(list);
    }
  }

  bool Command::check_parameters(serialize::Parameters &parameters,
                                 std::string &msg) const
  {
    if (parameters_.size() != parameters.size()) {
      std::ostringstream oss;
      oss << "Wrong parameter count. Expected ";
      oss <<  parameters_.size() << " but received " << parameters.size();
      msg = oss.str();
      return false;
    }

    for (size_t i = 0, s = parameters.size(); i < s; i++) {
      serialize::Type p_ = parameters_[i].type();
      serialize::Type p = parameters[i]->type();

      // check for DATASTRING type
      if (p_ == serialize::DATASTRING && p == serialize::STRING) {
        p = serialize::DATASTRING;
      }

      if (p != serialize::NIL && p != p_) {
        // check if list of multi-parameter was sent
        if (parameters_[i].multiple() && p == serialize::LIST) {
          std::vector<serialize::ParameterPtr> parts;
          if (parameters[i]->children(parts)) {
            std::vector<serialize::ParameterPtr>::iterator it;
            for (it = parts.begin(); it != parts.end(); ++it) {
              if ((**it).type() != p_) {
                std::ostringstream oss;
                oss << "Error at parameter [" << i << ":" << parameters_[i].name() << "]: ";
                oss << "Multiple parameter values have to be the same required type ";
                oss << xmlrpc::type_string(serialize::to_xml_type(p_));
                oss << ".";
                msg = oss.str();
                return false;
              }
            }
            // all partial values are of required type
            continue;
          }
        }
        // if not the type is not matching
        std::ostringstream oss;
        oss << "Error at parameter [" << i << ":" << parameters_[i].name() << "]: ";
        oss << "Parameter of type ";
        oss << xmlrpc::type_string(serialize::to_xml_type(p));;
        oss << " was given. Expected type ";
        oss << xmlrpc::type_string(serialize::to_xml_type(p_));
        oss << ".";
        msg = oss.str();
        return false;
      }
      // convert single parameters to list of length one for convenience
      else if (p != serialize::NIL && parameters_[i].multiple()) {
        serialize::ParameterPtr single = parameters[i];
        parameters[i] = serialize::ParameterPtr(new serialize::ListParameter);
        parameters[i]->add_child(single);
      }
    }

    return true;
  }

  const Command *Command::get_command(const std::string &name)
  {
    if (!commands_) {
      return 0;
    }
    std::map<std::string, Command *>::iterator it = commands_->find(name);

    if (it == commands_->end()) {
      return 0 ;
    }
    return it->second;
  }


  bool read_metadata(const serialize::ParameterPtr &map,
                     datatypes::Metadata &metadata, std::string &msg)
  {
    if (map->isNull()) {
      return true;
    }
    std::map<std::string, serialize::ParameterPtr> map_;
    if (!map->children(map_)) {
      msg = "unable to read metadata";
      return false;
    }
    std::map<std::string, serialize::ParameterPtr>::iterator mit;
    for (mit = map_.begin(); mit != map_.end(); ++mit) {
      metadata[mit->first] = mit->second->as_string();
    }
    return true;
  }


  bool extract_items(std::vector<serialize::ParameterPtr> input_list, size_t position, serialize::Type type,
                     serialize::Parameters &result)
  {
    result.set_as_array(true);

    for (auto input : input_list) {
      if (input->type() != serialize::LIST) {
        result.add_error(Error::m(ERR_INVALID_INPUT_TYPE, serialize::type_name(input->type()), serialize::type_name(serialize::LIST)));
        return false;
      }

      std::vector<serialize::ParameterPtr> input_items;
      input->children(input_items);

      if (input_items.size() != 2 && input_items.size() != 3) {
        result.add_error(Error::m(ERR_INVALID_INPUT_SUB_ITEM_SIZE, input_items.size(), "2 or 3"));
        return false;
      }

      if (input_items[position]->type() != type) {
        result.add_error(Error::m(ERR_INVALID_INPUT_SUB_ITEM_TYPE, position, serialize::type_name(input_items[position]->type()), serialize::type_name(type)));
        return false;
      }

      if (type == serialize::STRING) {
        result.add_string(input_items[position]->as_string());
      } else {
        result.add_error("invalid type... TODO: it must be implemented");
        return false;
      }
    }
    return true;
  }

  serialize::ParameterPtr build_command_info(const Command* command)
  {
    // Occult "administration" commands
    CommandDescription desc = command->description();


    serialize::ParameterPtr cmd_info(new serialize::MapParameter);

    // add parameter info
    serialize::ParameterPtr params_info(new serialize::ListParameter());
    const Parameters command_params = command->parameters();

    // add information about each parameter of the command
    Parameters::const_iterator pit;
    for (pit = command_params.begin(); pit != command_params.end(); ++pit) {
      serialize::ParameterPtr param_info(new serialize::ListParameter);

      param_info->add_child(serialize::ParameterPtr(
                              new serialize::SimpleParameter(pit->name()) ));

      param_info->add_child(serialize::ParameterPtr(
                              new serialize::SimpleParameter(xmlrpc::type_string(serialize::to_xml_type(pit->type()))) ));
      param_info->add_child(serialize::ParameterPtr(
                              new serialize::SimpleParameter(pit->multiple()) ));
      param_info->add_child(serialize::ParameterPtr(
                              new serialize::SimpleParameter(pit->description()) ));

      params_info->add_child(param_info);
    }
    cmd_info->add_child("parameters", params_info);


    // add result info
    serialize::ParameterPtr results_info(new serialize::ListParameter());
    const Parameters command_results = command->results();

    // add information about each resultof the command
    Parameters::const_iterator rit;
    for (rit = command_results.begin(); rit != command_results.end(); ++rit) {
      serialize::ParameterPtr result_info(new serialize::ListParameter);

      result_info->add_child(serialize::ParameterPtr(
                               new serialize::SimpleParameter(rit->name()) ));

      result_info->add_child(serialize::ParameterPtr(
                               new serialize::SimpleParameter(xmlrpc::type_string(serialize::to_xml_type(rit->type()))) ));
      result_info->add_child(serialize::ParameterPtr(
                               new serialize::SimpleParameter(rit->multiple()) ));
      result_info->add_child(serialize::ParameterPtr(
                               new serialize::SimpleParameter(rit->description()) ));

      results_info->add_child(result_info);
    }
    cmd_info->add_child("results", results_info);


    // add description
    serialize::ParameterPtr description(new serialize::ListParameter);
    description->add_child(serialize::ParameterPtr(
                             new serialize::SimpleParameter(serialize::STRING, desc.category.name)) );
    description->add_child(serialize::ParameterPtr(
                             new serialize::SimpleParameter(serialize::STRING, desc.category.description)) );
    description->add_child(serialize::ParameterPtr(
                             new serialize::SimpleParameter(serialize::STRING, desc.description)) );

    cmd_info->add_child("description", description);

    return cmd_info;
  }
}
