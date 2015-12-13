//
//  commands.cpp
//  epidb
//
//  Created by Felipe Albrecht on 29.05.13.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
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

  bool Command::checks(const std::string &user_key, std::string &msg)
  {
    if (!dba::is_initialized()) {
      msg = "The system was not initialized.";
      return false;
    }

    if (!dba::exists::user_by_key(user_key)) {
      msg = Error::m(ERR_INVALID_USER_KEY);
      return false;
    }
    return true;
  }

  void Command::set_id_names_return(const std::vector<utils::IdName> &id_names, serialize::Parameters &result) const
  {
    result.set_as_array(true);
    for(const utils::IdName & id_name: id_names) {
      std::vector<serialize::ParameterPtr> list;
      list.push_back(serialize::ParameterPtr(new serialize::SimpleParameter(serialize::STRING, id_name.id)));
      list.push_back(serialize::ParameterPtr(new serialize::SimpleParameter(serialize::STRING, id_name.name)));
      result.add_list(list);
    }
  }

  void Command::set_id_names_count_return(const std::vector<utils::IdNameCount> &id_name_counts, serialize::Parameters &result) const
  {
    result.set_as_array(true);
    for(const utils::IdNameCount & id_name_count: id_name_counts) {
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
}
