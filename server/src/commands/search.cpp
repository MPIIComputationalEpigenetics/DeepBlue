//
//  search.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 30.07.13.
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
#include <sstream>

#include "../dba/dba.hpp"
#include "../dba/collections.hpp"
#include "../dba/full_text.hpp"
#include "../dba/list.hpp"

#include "../datatypes/user.hpp"

#include "../extras/serialize.hpp"

#include "../engine/commands.hpp"

#include "../errors.hpp"

namespace epidb {
  namespace command {

    class SearchCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        std::string desc = "Search all data of all types for the given keyword. ";
        desc += "A minus (-) character in front of a keyword searches for data ";
        desc += "without the given keyword. ";
        desc += "The search can be restricted to the following data types are: ";
        desc += utils::vector_to_string(utils::capitalize_vector(dba::Collections::valid_search_Collections()), ", ");
        desc += ".";

        return CommandDescription(categories::GENERAL_INFORMATION, desc);
      }

      static  Parameters parameters_()
      {
        Parameter p[] = {
          Parameter("keyword", serialize::STRING, "keyword to search by"),
          Parameter("type", serialize::STRING, "type of data to search for - " + utils::vector_to_string(utils::capitalize_vector(dba::Collections::valid_search_Collections()), ", "), true),
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 3);
        return params;
      }

      static Parameters results_()
      {
        Parameter p[] = {
          Parameter("results", serialize::LIST, "search results as [id, name, type]")
        };
        Parameters results(&p[0], &p[0] + 1);
        return results;
      }

    public:
      SearchCommand() : Command("search", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        const std::string keyword = parameters[0]->as_string();
        const std::string user_key = parameters[2]->as_string();

        std::string msg;
        datatypes::User user;

        if (!check_permissions(user_key, datatypes::LIST_COLLECTIONS, user, msg )) {
          result.add_error(msg);
          return false;
        }

        std::vector<serialize::ParameterPtr> types;
        parameters[1]->children(types);



        std::vector<std::string> types_s;
        for (std::vector<serialize::ParameterPtr>::iterator it = types.begin(); it != types.end(); ++it) {
          std::string type = utils::lower_case((**it).as_string());
          if (!dba::Collections::is_valid_search_collection(type)) {
            msg = Error::m(ERR_INVALID_COLLECTION_NAME, type, utils::vector_to_string(utils::capitalize_vector(dba::Collections::valid_search_Collections()), ", "));
            result.add_error(msg);
            return false;
          }
          types_s.push_back(type);
        }

        std::vector<utils::IdName> private_projects_id_names;
        if (!dba::list::private_projects(user, private_projects_id_names, msg)) {
          result.add_error(msg);
          return false;
        }

        std::vector<std::string> private_projects;
        for (const auto& project : private_projects_id_names) {
          private_projects.push_back(utils::normalize_name(project.name));
        }

        std::vector<dba::search::TextSearchResult> search_res;
        if (!dba::search::search_full_text(keyword, types_s, private_projects, search_res, msg)) {
          result.add_error(msg);
          return false;
        }

        result.set_as_array(true);
        std::vector<dba::search::TextSearchResult>::iterator it;
        for (it = search_res.begin(); it != search_res.end(); ++it) {
          std::vector<serialize::ParameterPtr> list;
          list.push_back(serialize::ParameterPtr(new serialize::SimpleParameter(serialize::STRING, it->id)));
          list.push_back(serialize::ParameterPtr(new serialize::SimpleParameter(serialize::STRING, it->name)));
          list.push_back(serialize::ParameterPtr(new serialize::SimpleParameter(serialize::STRING, it->type)));
          result.add_list(list);
        }

        return true;
      }
    } searchCommand;
  }
}
