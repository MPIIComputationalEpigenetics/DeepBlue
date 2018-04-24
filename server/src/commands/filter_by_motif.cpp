//
//  filter_by_motif.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 14.03.18.
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

#include "../datatypes/user.hpp"

#include "../dba/dba.hpp"
#include "../dba/exists.hpp"
#include "../dba/queries.hpp"

#include "../engine/commands.hpp"

#include "../extras/utils.hpp"
#include "../extras/serialize.hpp"

#include "../errors.hpp"

namespace epidb {
  namespace command {

    class FilterByMotifCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::OPERATIONS, "Filter the genomic regions by a regular expression motif.");
      }

      static Parameters parameters_()
      {
        return {
          parameters::QueryId,
          Parameter("motif", serialize::STRING, "motif (PERL regular expression)"),
          parameters::UserKey
        };
      }

      static Parameters results_()
      {
        return {
          Parameter("id", serialize::STRING, "id of filtered query")
        };
      }

    public:
      FilterByMotifCommand() : Command("filter_by_motif", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        const std::string query_id = parameters[0]->as_string();
        const std::string motif = parameters[1]->as_string();
        const std::string user_key = parameters[2]->as_string();

        std::string msg;
        datatypes::User user;

        if (!check_permissions(user_key, datatypes::GET_DATA, user, msg )) {
          result.add_error(msg);
          return false;
        }

        if (!dba::exists::query(user, query_id, msg)) {
          result.add_error("Invalid query id: '" + query_id + "'" + msg);
          return false;
        }

        if (motif.empty()) {
          result.add_error(Error::m(ERR_USER_MOTIF_MISSING));
          return false;
        }

        mongo::BSONObjBuilder args_builder;
        args_builder.append("query", query_id);
        args_builder.append("motif", motif);

        std::string filtered_query_id;
        if (!dba::query::store_query(user, "filter_by_motif", args_builder.obj(), filtered_query_id, msg)) {
          result.add_error(msg);
          return false;
        }

        result.add_string(filtered_query_id);
        return true;
      }

    } filterByMotifCommand;
  }
}