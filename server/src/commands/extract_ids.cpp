//
//  extract_ids.cpp
//  epidb
//
//  Created by Felipe Albrecht on 07.01.16.
//  Copyright (c) 2016 Max Planck Institute for Informatics. All rights reserved.
//

#include "../engine/commands.hpp"

#include "../extras/serialize.hpp"

#include "../errors.hpp"

namespace epidb {
  namespace command {

    class ExtractIdsCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::UTILITIES, "Extract the names from a list of ID and Names.");
      }

      static Parameters parameters_()
      {
        Parameter p[] = {
          Parameter("list", serialize::LIST, "list of lists of IDs and Names")
        };
        Parameters params(&p[0], &p[0] + 1);
        return params;
      }

      static Parameters results_()
      {
        Parameter p[] = {
          Parameter("ids", serialize::LIST, "list containing the extracted IDs")
        };
        Parameters results(&p[0], &p[0] + 1);
        return results;
      }

    public:
      ExtractIdsCommand() : Command("extract_ids", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        std::vector<serialize::ParameterPtr> input_list;
        parameters[0]->children(input_list);

        return extract_items(input_list, 0, serialize::STRING, result);
      }
    } extractIdsCommand;
  }
}
