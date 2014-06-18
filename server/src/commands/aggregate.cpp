//
//  aggregate.cpp
//  epidb
//
//  Created by Felipe Albrecht on 10.04.14.
//  Copyright (c) 2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include <sstream>
#include <vector>
#include <map>

#include <boost/algorithm/string.hpp>

#include "../dba/dba.hpp"
#include "../dba/helpers.hpp"
#include "../dba/queries.hpp"

#include "../engine/commands.hpp"

#include "../extras/utils.hpp"
#include "../extras/serialize.hpp"

#include "../log.hpp"
#include "../regions.hpp"

namespace epidb {
  namespace command {

    class AggregateCommand: public Command {

      typedef std::vector<std::pair<std::string, std::string> > Format;

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::OPERATIONS, "Summarize the data regions content in range regions. Use the fields @AGG.MIN, @AGG.MAX, @AGG.MEDIAN, @AGG.MEAN, @AGG.VAR, @AGG.SD, @AGG.COUNT in the get_regions command format parameter for retrieving the computed values."); 
      }

      static Parameters parameters_()
      {
        Parameter p[] = {
          Parameter("data_id", serialize::STRING, "id of the query with the data"),
          Parameter("ranges_id", serialize::STRING, "id of the query with the regions range"),
          Parameter("field", serialize::STRING, "id of the aggregate regions range"),
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 4);
        return params;
      }

      static Parameters results_()
      {
        Parameter p[] = {
          Parameter("regions", serialize::STRING, "BED formated regions")
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
        bool ok = false;
        if (!dba::check_user(user_key, ok, msg)) {
          result.add_error(msg);
          return false;
        }
        if (!ok) {
          result.add_error("Invalid user key.");
          return false;
        }

        if (!dba::check_query(user_key, data_id, ok, msg)) {
          result.add_error(msg);
          return false;
        }
        if (!ok) {
          result.add_error("Invalid data query id.");
          return false;
        }

        if (!dba::check_query(user_key, ranges_id, ok, msg)) {
          result.add_error(msg);
          return false;
        }
        if (!ok) {
          result.add_error("Invalid regions query id.");
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
