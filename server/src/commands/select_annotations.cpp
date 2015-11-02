//
//  select_annotation.cpp
//  epidb
//
//  Created by Felipe Albrecht on 28.08.13.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
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

    class SelectAnnotationsCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::OPERATIONS, "Selects annotation regions matching the given parameters.");
      }

      static Parameters parameters_()
      {
        Parameter p[] = {
          Parameter("annotation_name", serialize::STRING, "name(s) of selected annotation(s)", true),
          parameters::GenomeMultiple,
          Parameter("chromosome", serialize::STRING, "chromosome name(s)", true),
          Parameter("start", serialize::INTEGER, "minimum start region"),
          Parameter("end", serialize::INTEGER, "maximum end region"),
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 6);
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
      SelectAnnotationsCommand() : Command("select_annotations", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        std::vector<serialize::ParameterPtr> annotations;
        std::vector<serialize::ParameterPtr> genomes;
        std::vector<serialize::ParameterPtr> chromosomes;
        parameters[0]->children(annotations);
        parameters[1]->children(genomes);
        parameters[2]->children(chromosomes);

        const std::string user_key = parameters[5]->as_string();

        std::string msg;
        datatypes::User user;

        if (!check_permissions(user_key, datatypes::GET_DATA, user, msg )) {
          result.add_error(msg);
          return false;
        }

        if (annotations.size() == 0) {
          result.add_error(Error::m(ERR_USER_ANNOTATION_MISSING));
          return false;
        }

        if (genomes.size() == 0) {
          result.add_error(Error::m(ERR_USER_GENOME_MISSING));
          return false;
        }

        mongo::BSONObjBuilder args_builder;
        if (annotations.size() > 0) {
          args_builder.append("annotation", utils::build_array(annotations));
          args_builder.append("norm_annotation", utils::build_annotation_normalized_array(annotations));
        }

        const int start = parameters[3]->isNull() ? -1 : parameters[3]->as_long();
        const int end = parameters[4]->isNull() ? -1 : parameters[4]->as_long();

        if (start > 0) {
          args_builder.append("start", (int) start);
        }
        if (end > 0) {
          args_builder.append("end", (int) end);
        }

        std::vector<std::string> genomes_s;
        std::vector<std::string> norm_genomes_s;
        std::vector<serialize::ParameterPtr>::iterator git;
        for (git = genomes.begin(); git != genomes.end(); ++git) {
          std::string genome = (*git)->as_string();
          std::string norm_genome = utils::normalize_name(genome);
          genomes_s.push_back(genome);
          norm_genomes_s.push_back(norm_genome);
        }

        std::set<std::string> chroms;
        if (chromosomes.size() == 0) {
          if (!dba::genomes::get_chromosomes(genomes_s, chroms, msg)) {
            result.add_error(msg);
            return false;
          }
        } else {
          std::vector<serialize::ParameterPtr>::iterator it;
          for (it = chromosomes.begin(); it != chromosomes.end(); ++it) {
            chroms.insert((**it).as_string());
          }
        }
        args_builder.append("chromosomes", chroms);
        args_builder.append("genomes", genomes_s);
        args_builder.append("norm_genomes", norm_genomes_s);

        std::string query_id;
        if (!dba::query::store_query("annotation_select", args_builder.obj(), user_key, query_id, msg)) {
          result.add_error(msg);
          return false;
        }

        result.add_string(query_id);
        return true;
      }
    } selectAnnotationsCommand;
  }
}
