//
//  extract_ids.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 07.01.16.
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

#include "../engine/commands.hpp"

#include "../extras/serialize.hpp"

#include "../errors.hpp"

namespace epidb {
  namespace command {

    class ExtractIdsCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::UTILITIES, "An utility command that returns a list of IDs extracted from a list of ID and names.");
      }

      static Parameters parameters_()
      {
        Parameter p[] = {
          Parameter("list", serialize::LIST, "list of lists of IDs and names")
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
