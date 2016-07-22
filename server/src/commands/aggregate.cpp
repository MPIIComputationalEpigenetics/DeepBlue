//
//  aggregate.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 10.04.14.
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

#include <sstream>
#include <vector>
#include <map>

#include <boost/algorithm/string.hpp>

#include "../datatypes/regions.hpp"

#include "../dba/dba.hpp"
#include "../dba/exists.hpp"
#include "../dba/helpers.hpp"
#include "../dba/queries.hpp"
#include "../datatypes/user.hpp"
#include "../engine/commands.hpp"

#include "../extras/utils.hpp"
#include "../extras/serialize.hpp"

#include "../log.hpp"
#include "../errors.hpp"

namespace epidb {
  namespace command {

    class AggregateCommand: public Command {

      typedef std::vector<std::pair<std::string, std::string> > Format;

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::OPERATIONS, "Summarize the data_id content using the regions specified in ranges_id as boundaries. Use the fields @AGG.MIN, @AGG.MAX, @AGG.MEDIAN, @AGG.MEAN, @AGG.VAR, @AGG.SD, @AGG.COUNT in 'get_regions' command 'format' parameter to retrieve the computed values minimum, maximum, median, mean, variance, standard deviation and number of regions, respectively.");
      }

      static Parameters parameters_()
      {
        Parameter p[] = {
          Parameter("data_id", serialize::STRING, "id of the query with the data"),
          Parameter("ranges_id", serialize::STRING, "id of the query with the regions range"),
          Parameter("column", serialize::STRING, "name of the column that will be used in the aggregation"),
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 4);
        return params;
      }

      static Parameters results_()
      {
        Parameter p[] = {
          Parameter("regions", serialize::STRING, "query id of this aggregation operation")
        };
        Parameters results(&p[0], &p[0] + 1);
        return results;
      }

    public:
      AggregateCommand() : Command("aggregate", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        const std::string data_id = parameters[0]->as_string();
        const std::string ranges_id = parameters[1]->as_string();
        const std::string field = parameters[2]->as_string();
        const std::string user_key = parameters[3]->as_string();

        std::string msg;
        datatypes::User user;

        if (!check_permissions(user_key, datatypes::GET_DATA, user, msg )) {
          result.add_error(msg);
          return false;
        }

        if (!dba::exists::query(data_id, user_key, msg)) {
          result.add_error("Invalid data query id." + msg);
          return false;
        }

        if (!dba::exists::query(ranges_id, user_key, msg)) {
          result.add_error("Invalid regions query id." + msg);
          return false;
        }

        if (field.empty()) {
          result.add_error("No field was defined. Please, define one field that will be used for the aggregation operations.");
          return false;
        }

        mongo::BSONObjBuilder args_builder;
        args_builder.append("data_id", data_id);
        args_builder.append("ranges_id", ranges_id);
        args_builder.append("field", field);

        std::string aggregate_query_id;
        if (!dba::query::store_query("aggregate", args_builder.obj(), user_key, aggregate_query_id, msg)) {
          result.add_error(msg);
          return false;
        }

        result.add_string(aggregate_query_id);

        return true;
      }
    } aggregateCommand;
  }
}
