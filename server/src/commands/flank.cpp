//
//  flank.cpp
//  epidb
//
//  Created by Fabian Reinartz on 29.08.15.
//  Copyright (c) 2016 Max Planck Institute for Informatics. All rights reserved.
//

#include "../dba/dba.hpp"
#include "../dba/exists.hpp"
#include "../dba/queries.hpp"

#include "../datatypes/user.hpp"

#include "../engine/commands.hpp"

#include "../extras/serialize.hpp"

#include "../errors.hpp"

namespace epidb {
  namespace command {

    class FlankCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::OPERATIONS, "Generate flanking regions for the given regions.");
      }

      static Parameters parameters_()
      {
        Parameter p[] = {
          Parameter("query_id", serialize::STRING, "id of the query that contains the regions"),
          Parameter("start", serialize::INTEGER, "Number of base pairs after the end of the region. Use a negative number to denote the number of base pairs before the start of the region."),
          Parameter("length", serialize::INTEGER, "The new region length"),
          Parameter("use_strand", serialize::BOOLEAN, "Use the region column STRAND to define the region direction"),
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 5);
        return params;
      }

      static Parameters results_()
      {
        Parameter p[] = {
          Parameter("id", serialize::STRING, "id of the new query")
        };
        Parameters results(&p[0], &p[0] + 1);
        return results;
      }

    public:
      FlankCommand() : Command("flank", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        const std::string flanked_query_id = parameters[0]->as_string();
        const int start = parameters[1]->isNull() ? -1 : parameters[1]->as_long();
        const int length = parameters[2]->isNull() ? -1 : parameters[2]->as_long();
        const bool use_strand = parameters[3]->as_boolean();
        const std::string user_key = parameters[4]->as_string();

        std::string msg;
        datatypes::User user;

        if (!check_permissions(user_key, datatypes::GET_DATA, user, msg )) {
          result.add_error(msg);
          return false;
        }

        if (!dba::exists::query(flanked_query_id, user_key, msg)) {
          if (msg.empty()) {
            result.add_error(Error::m(ERR_INVALID_QUERY_ID, flanked_query_id));
          } else {
            result.add_error(msg);
          }
          return false;
        }

        mongo::BSONObjBuilder args_builder;
        args_builder.append("query_id", flanked_query_id);
        args_builder.append("start", start);
        args_builder.append("length", length);
        args_builder.append("use_strand", use_strand);

        std::string query_id;
        if (!dba::query::store_query("flank", args_builder.obj(), user_key, query_id, msg)) {
          result.add_error(msg);
          return false;
        }

        result.add_string(query_id);
        return true;
      }

    } flankCommand;
  }
}



