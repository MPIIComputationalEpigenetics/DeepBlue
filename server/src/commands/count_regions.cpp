//
//  count_regions.cpp
//  epidb
//
//  Created by Felipe Albrecht on 29.08.13.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include <sstream>
#include <map>

#include "../dba/dba.hpp"
#include "../dba/queries.hpp"

#include "../extras/serialize.hpp"

#include "../engine/commands.hpp"

#include "../errors.hpp"

namespace epidb {
  namespace command {

    class CountRegionsCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::OPERATIONS, "Counts the number of regions in the result of the given query.");
      }

      static  Parameters parameters_()
      {
        Parameter p[] = {
          Parameter("query_id", serialize::STRING, "id of the counted query"),
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 2);
        return params;
      }

      static Parameters results_()
      {
        Parameter p[] = {
          Parameter("size", serialize::INTEGER, "number of regions")
        };
        Parameters results(&p[0], &p[0] + 1);
        return results;
      }

    public:
      CountRegionsCommand() : Command("count_regions", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        const std::string query_id = parameters[0]->as_string();
        const std::string user_key = parameters[1]->as_string();

        std::string msg;
        if (!Command::checks(user_key, msg)) {
          result.add_error(msg);
          return false;
        }

        bool ok = false;
        if (!dba::check_query(user_key, query_id, ok, msg)) {
          result.add_error(msg);
          return false;
        }
        if (!ok) {
          result.add_error("Invalid query id: '" + query_id + "'");
          return false;
        }

        size_t size = 0;
        if (!dba::query::count_regions(user_key, query_id, size, msg)) {
          result.add_error(msg);
          return false;
        }

        result.add_int(size);

        return true;
      }
    } countRegionsCommand;
  }
}
