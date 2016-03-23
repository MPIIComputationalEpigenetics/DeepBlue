//
//  datatable.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 10.03.2016.
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

#include "../dba/datatable.hpp"
#include "../dba/exists.hpp"
#include "../dba/users.hpp"

#include "../extras/serialize.hpp"

#include "../engine/commands.hpp"

namespace epidb {
  namespace command {

    class DatatableCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::ADMINISTRATION, "interface for using DeepBlue with Datatables.");
      }

      static Parameters parameters_()
      {
        Parameter p[] = {
          Parameter("collection", serialize::STRING, "name(s) of selected experiment(s)"),
          Parameter("columns", serialize::STRING, "name(s) of selected epigenetic mark(s)", true),
          Parameter("start", serialize::INTEGER, "minimum start region"),
          Parameter("length", serialize::INTEGER, "maximum end region"),
          Parameter("global_search", serialize::STRING, "id(s) of selected sample(s)"),
          Parameter("sort_column", serialize::STRING, "id(s) of selected sample(s)"),
          Parameter("sort_direction", serialize::STRING, "id(s) of selected sample(s)"),
          Parameter("has_filter", serialize::BOOLEAN, "name(s) of selected technique(es)"),
          Parameter("columns_filters", serialize::MAP, "name(s) of selected technique(es)"),
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 10);
        return params;
      }

      static Parameters results_()
      {
        Parameter p[] = {
          Parameter("total_size", serialize::INTEGER, "total of items that match with the given input"),
          Parameter("rows", serialize::LIST, "list of rows")
        };
        Parameters results(&p[0], &p[0] + 1);
        return results;
      }

    public:
      DatatableCommand() : Command("datatable", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string& ip,
                       const serialize::Parameters& parameters, serialize::Parameters& result) const
      {
        const std::string collection = parameters[0]->as_string();

        std::vector<serialize::ParameterPtr> columns_parameters;
        parameters[1]->children(columns_parameters);
        std::vector<std::string> columns = utils::build_vector(columns_parameters);

        const int start = parameters[2]->isNull() ? -1 : parameters[2]->as_long();
        const int length = parameters[3]->isNull() ? -1 : parameters[3]->as_long();
        const std::string global_search = parameters[4]->as_string();
        const std::string sort_column = parameters[5]->as_string();
        const std::string sort_direction = parameters[6]->as_string();
        const bool has_filter = parameters[7]->as_boolean();

        std::string msg;

        // Here we use Metadata typedef just for simplicity
        datatypes::Metadata columns_filters;
        if (!read_metadata(parameters[8], columns_filters, msg)) {
          result.add_error(msg);
          return false;
        }

        const std::string user_key = parameters[9]->as_string();

        utils::IdName user;
        if (!dba::exists::user_by_key(user_key)) {
          user.name = "a Stranger";
        } else {
          if (!dba::users::get_user(user_key, user, msg)) {
            result.add_error(msg);
            return false;
          }
        }

        size_t size;
        std::vector<std::vector<std::string>> results;

        if (!dba::datatable::datatable(collection, columns, start, length,
                            global_search, sort_column, sort_direction,
                            has_filter,  columns_filters,
                            user_key, size, results, msg)) {
          result.add_error(msg);
          return false;
        }

        serialize::ParameterPtr table_data(new serialize::ListParameter());
        for (const auto& result_row : results) {
          std::vector<serialize::ParameterPtr> row;
          for (const auto& cell : result_row) {
            row.push_back(serialize::ParameterPtr(new serialize::SimpleParameter(serialize::STRING, std::move(cell))));
          }
          table_data->add_child(serialize::ParameterPtr(new serialize::ListParameter(row)));
        }

        result.add_int(size);
        result.add_list(table_data);

        return true;
      }

    } atatableCommand;
  }
}
