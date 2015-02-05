//
//  get_experiments_by_query.cpp
//  epidb
//
//  Created by Felipe Albrecht on 12.06.13.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include "../dba/dba.hpp"

#include "../engine/commands.hpp"
#include "../engine/engine.hpp"

#include "../extras/serialize.hpp"

#include "../errors.hpp"
#include "../log.hpp"


namespace epidb {
  namespace command {

    class GetExperimentsByQueryCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::OPERATIONS, "Return a list of experiments and annotations that have at least one region in the data set represented by the query.");
      }

      static Parameters parameters_()
      {
        Parameter p[] = {
          Parameter("query_id", serialize::STRING, "id of the query"),
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 2);
        return params;
      }

      static Parameters results_()
      {
        Parameter p[] = {
          Parameter("experiments", serialize::LIST, "List containing experiments names and ids")
        };
        Parameters results(&p[0], &p[0] + 1);
        return results;
      }

    public:
      GetExperimentsByQueryCommand() : Command("get_experiments_by_query", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        const std::string query_id = parameters[0]->as_string();
        const std::string user_key = parameters[1]->as_string();

        bool ok = false;
        std::string msg;
        if (!Command::checks(user_key, msg)) {
          result.add_error(msg);
          return false;
        }

        if (!dba::check_query(user_key, query_id, ok, msg)) {
          result.add_error(msg);
          return false;
        }
        if (!ok) {
          result.add_error("Invalid query id.");
          return false;
        }

        std::string request_id;
        if (!epidb::Engine::instance().queue_get_experiments_by_query(query_id, user_key, request_id, msg)) {
          result.add_error(msg);
          return false;
        }

        result.add_string(request_id);
        return true;

      }
    } getExperimentsByQueryCommand;
  }
}
