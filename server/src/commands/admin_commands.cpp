//
//  list_commands.cpp
//  epidb
//
//  Created by Felipe Albrecht on 07.01.16.
//  Copyright (c) 2016 Max Planck Institute for Informatics. All rights reserved.
//

#include "../dba/dba.hpp"

#include "../engine/commands.hpp"

#include "../extras/serialize.hpp"
#include "../extras/utils.hpp"

namespace epidb {
  namespace command {

    class AdminCommandsCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::ADMINISTRATION, "Lists all existing commands.");
      }

      static Parameters parameters_()
      {
        Parameter p[] = {};
        Parameters params(&p[0], &p[0]);
        return params;
      }

      static Parameters results_()
      {
        Parameter p[] = {
          Parameter("admin_commands", serialize::MAP, "command descriptions")
        };
        Parameters results(&p[0], &p[0] + 1);
        return results;
      }

    public:
      AdminCommandsCommand() : Command("admin_commands", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        serialize::ParameterPtr commands_info(new serialize::MapParameter);

        // add information about every command
        std::map<std::string, Command *>::const_iterator it;
        for (it = commands_->begin(); it != commands_->end(); ++it) {
          CommandDescription desc = it->second->description();
          if (desc.category == categories::ADMINISTRATION) {
            auto cmd_info = build_command_info(it->second);
            commands_info->add_child(it->first, cmd_info);
          }
        }

        result.add_param(commands_info);
        return true;
      }

    } adminCommandsCommand;
  }
}