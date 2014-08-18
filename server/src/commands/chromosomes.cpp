//
//  chromosomes.cpp
//  epidb
//
//  Created by Felipe Albrecht on 10.10.14.
//  Copyright (c) 2014 Max Planck Institute for Computer Science. All rights reserved.
//
#include <boost/foreach.hpp>

#include "../dba/dba.hpp"
#include "../dba/genomes.hpp"
#include "../dba/helpers.hpp"

#include "../engine/commands.hpp"

#include "../extras/utils.hpp"

#include "../errors.hpp"

namespace epidb {
  namespace command {

    class ChromosomesCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::GENOMES, "List all chromosomes of a given genome.");
      }

      static Parameters parameters_()
      {
        Parameter p[] = {
          parameters::Genome,
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 2);
        return params;
      }

      static Parameters results_()
      {
        Parameter p[] = {
          Parameter("chromosomes", serialize::LIST, "A list containing all chromosomes, with theirs names and sizes")
        };
        Parameters results(&p[0], &p[1]);
        return results;
      }

    public:
      ChromosomesCommand() : Command("chromosomes", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        const std::string genome = parameters[0]->as_string();
        const std::string user_key = parameters[1]->as_string();

        std::string msg;
        bool ok;
        if (!dba::check_user(user_key, ok, msg)) {
          result.add_error(msg);
          return false;
        }
        if (!ok) {
          std::string s = Error::m(ERR_INVALID_USER_KEY);
          result.add_error(s);
          return false;
        }

        std::vector<dba::genomes::ChromosomeInfo> chromosomes;
        if (!dba::genomes::get_chromosomes(genome, chromosomes, msg)) {
          result.add_error(msg);
          return false;
        }

        result.set_as_array(true);
        BOOST_FOREACH(dba::genomes::ChromosomeInfo info, chromosomes) {
          std::vector<serialize::ParameterPtr> list;
          list.push_back(serialize::ParameterPtr(new serialize::SimpleParameter(info.name)));
          list.push_back(serialize::ParameterPtr(new serialize::SimpleParameter((long long) info.size)));
          result.add_list(list);
        }

        return true;
      }
    } chromosomesCommand;
  }
}
