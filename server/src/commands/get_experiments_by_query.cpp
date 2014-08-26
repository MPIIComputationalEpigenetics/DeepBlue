//
//  get_experiments_by_query.cpp
//  epidb
//
//  Created by Felipe Albrecht on 12.06.13.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include <sstream>
#include <map>

#include "../dba/dba.hpp"
#include "../dba/queries.hpp"

#include "../engine/commands.hpp"

#include "../extras/utils.hpp"
#include "../extras/serialize.hpp"

namespace epidb {
    namespace command {

      class GetExperimentsByQueryCommand: public Command {

      private:
        static CommandDescription desc_() {
          return CommandDescription(categories::OPERATIONS, "Return a list of experiments and annotations that have at least one region in the data set represented by the query.");
        }

        static Parameters parameters_() {
          Parameter p[] = {
            Parameter("query_id", serialize::STRING, "id of the query"),
            parameters::UserKey
          };
          Parameters params(&p[0], &p[0]+2);
          return params;
        }

        static Parameters results_() {
          Parameter p[] = {
            Parameter("experiments", serialize::LIST, "List containing experiments names and ids")
          };
          Parameters results(&p[0], &p[0]+1);
          return results;
        }

      public:
        GetExperimentsByQueryCommand() : Command("get_experiments_by_query", parameters_(), results_(), desc_()) {}

        virtual bool run(const std::string& ip,
            const serialize::Parameters& parameters, serialize::Parameters& result) const
        {
          const std::string query_id = parameters[0]->as_string();
          const std::string user_key = parameters[1]->as_string();

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

          if (!dba::check_query(user_key, query_id, ok, msg)) {
            result.add_error(msg);
            return false;
          }
          if (!ok) {
            result.add_error("Invalid query id.");
            return false;
          }

          std::vector<utils::IdName> experiments_name;
          if (!dba::query::get_experiments_by_query(user_key, query_id, experiments_name, msg)) {
            result.add_error(msg);
            return false;
          }

          result.set_as_array(true);
          BOOST_FOREACH(utils::IdName id_name, experiments_name) {
            std::vector<serialize::ParameterPtr> list;
            list.push_back(serialize::ParameterPtr(new serialize::SimpleParameter(serialize::STRING, id_name.id)));
            list.push_back(serialize::ParameterPtr(new serialize::SimpleParameter(serialize::STRING, id_name.name)));
            result.add_list(list);
          }

          return true;
        }
      } getExperimentsByQueryCommand;
    }
}
