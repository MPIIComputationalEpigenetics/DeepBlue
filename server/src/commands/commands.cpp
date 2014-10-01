//
//  list_commands.cpp
//  epidb
//
//  Created by Fabian Reinartz on 17.12.13.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include "../dba/dba.hpp"

#include "../engine/commands.hpp"

#include "../extras/serialize.hpp"
#include "../extras/utils.hpp"

namespace epidb {
  namespace command {

    class CommandsCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::STATUS, "Lists all existing commands.");
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
          Parameter("commands", serialize::MAP, "command descriptions")
        };
        Parameters results(&p[0], &p[0] + 1);
        return results;
      }

    public:
      CommandsCommand() : Command("commands", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        serialize::ParameterPtr commands_info(new serialize::MapParameter);

        // add information about every command
        std::map<std::string, Command *>::const_iterator it;
        for (it = commands_->begin(); it != commands_->end(); ++it) {

          // Occult "administration" commands
          CommandDescription desc = it->second->description();
          if (desc.category == categories::ADMINISTRATION) {
            continue;
          }

          serialize::ParameterPtr cmd_info(new serialize::MapParameter);

          // add parameter info
          serialize::ParameterPtr params_info(new serialize::ListParameter());
          const Parameters command_params = it->second->parameters();

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
          const Parameters command_results = it->second->results();

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

          commands_info->add_child(it->first, cmd_info);
        }

        result.add_param(commands_info);
        return true;
      }

    } commandsCommand;
  }
}