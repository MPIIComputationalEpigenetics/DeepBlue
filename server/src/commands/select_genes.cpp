//
//  select_genes.cpp
//  epidb
//
//  Created by Felipe Albrecht on 10.09.15.
//  Copyright (c) 2015 Max Planck Institute for Computer Science. All rights reserved.
//

#include <sstream>
#include <map>
#include <set>

#include "../dba/dba.hpp"
#include "../dba/genomes.hpp"
#include "../dba/helpers.hpp"
#include "../dba/queries.hpp"

#include "../datatypes/user.hpp"

#include "../engine/commands.hpp"

#include "../extras/serialize.hpp"

#include "../errors.hpp"

namespace epidb {
  namespace command {

    class SelectGenesCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::GENES, "Selects genes as regions.");
      }

      static Parameters parameters_()
      {
        Parameter p[] = {
          Parameter("genes_name", serialize::STRING, "name(s) of selected genes(s)", true),
          Parameter("gene_set", serialize::STRING, "gene set name"),
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 3);
        return params;
      }

      static Parameters results_()
      {
        Parameter p[] = {
          Parameter("id", serialize::STRING, "query id")
        };
        Parameters results(&p[0], &p[0] + 1);
        return results;
      }

    public:
      SelectGenesCommand() : Command("select_genes", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        std::vector<serialize::ParameterPtr> genes;
        parameters[0]->children(genes);

        const std::string gene_set = parameters[1]->as_string();
        const std::string user_key = parameters[2]->as_string();

        std::string msg;
        datatypes::User user;

        if (!check_permissions(user_key, datatypes::GET_DATA, user, msg )) {
          result.add_error(msg);
          return false;
        }

        if (genes.empty()) {
          result.add_error(Error::m(ERR_USER_GENE_MISSING));
          return false;
        }

        if (gene_set.empty()) {
          result.add_error(Error::m(ERR_USER_GENE_SET_MISSING));
          return false;
        }

        mongo::BSONObjBuilder args_builder;
        args_builder.append("genes", dba::helpers::build_array(genes));
        args_builder.append("gene_set", utils::normalize_name(gene_set));

        std::string query_id;
        if (!dba::query::store_query("genes_select", args_builder.obj(), user_key, query_id, msg)) {
          result.add_error(msg);
          return false;
        }

        result.add_string(query_id);
        return true;
      }
    } selectGenesCommand;
  }
}
