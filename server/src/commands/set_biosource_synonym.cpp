//
//  set_biosource_synonym.cpp
//  epidb
//
//  Created by Felipe Albrecht on 01.08.13.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include "../dba/controlled_vocabulary.hpp"

#include "../engine/commands.hpp"

#include "../extras/serialize.hpp"

#include "../log.hpp"
#include "../errors.hpp"

namespace epidb {
  namespace command {

    class SetBioSourceSynonymCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::BIOSOURCE_RELATIONSHIP, "Sets a biosource synonym.");
      }

      static  Parameters parameters_()
      {
        Parameter p[] = {
          Parameter("biosource", serialize::STRING, "name of the biosource"),
          Parameter("synonym_name", serialize::STRING, "name of the synonym"),
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 3);
        return params;
      }

      static Parameters results_()
      {
        Parameter p[] = {
          Parameter("synonym_name", serialize::STRING, "inserted synonym_name")
        };
        Parameters results(&p[0], &p[0] + 1);
        return results;
      }

    public:
      SetBioSourceSynonymCommand() : Command("set_biosource_synonym", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        const std::string biosource_name = parameters[0]->as_string();
        const std::string synonym_name = parameters[1]->as_string();
        const std::string user_key = parameters[2]->as_string();

        std::string msg;
        if (!Command::checks(user_key, msg)) {
          result.add_error(msg);
          return false;
        }

        if (!dba::cv::set_biosource_synonym_complete(biosource_name, synonym_name, user_key, msg)) {
          result.add_error(msg);
          return false;
        }

        result.add_string(synonym_name);

        return true;
      }

    } setBioSourceSynonymCommand;
  }
}
