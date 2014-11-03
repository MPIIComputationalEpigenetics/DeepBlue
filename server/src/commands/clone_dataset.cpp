//
//  clone_dataset.cpp
//  epidb
//
//  Created by Felipe Albrecht on 01.10.2014.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include <string>

#include "../dba/dba.hpp"
#include "../dba/clone.hpp"

#include "../engine/commands.hpp"
#include "../extras/serialize.hpp"

namespace epidb {
  namespace command {

    class CloneDatasetCommand : public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::DATA_MODIFICATION,
                                  "Clone the dataset, allowing to change the description, column format (restrictively), and extra_metadata.");
      }

      static  Parameters parameters_()
      {
        Parameter p[] = {
          Parameter("dataset_id", serialize::STRING, "ID of the dataset (experiment or annotation ID)"),
          Parameter("new_name", serialize::STRING, "New dataset name"),
          Parameter("description", serialize::STRING, "description of the experiment - empty to copy from the cloned dataset"),
          Parameter("format", serialize::STRING, "format of the provided data - empty to copy from the cloned dataset"),
          Parameter("extra_metadata", serialize::MAP, "additional metadata - empty to copy from the cloned dataset"),
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 6);
        return params;
      }

      static Parameters results_()
      {
        Parameter p[] = {
          Parameter("id", serialize::STRING, "id of the new dataset")
        };
        Parameters results(&p[0], &p[0] + 1);
        return results;
      }

    public:
      CloneDatasetCommand() : Command("clone_dataset", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        const std::string id = parameters[0]->as_string();
        const std::string name = parameters[1]->as_string();
        const std::string description = parameters[2]->as_string();;
        const std::string format = parameters[3]->as_string();
        const std::string user_key = parameters[5]->as_string();

        std::string msg;
        if (!Command::checks(user_key, msg)) {
          result.add_error(msg);
          return false;
        }

        datatypes::Metadata extra_metadata;
        if (!read_metadata(parameters[4], extra_metadata, msg)) {
          result.add_error(msg);
          return false;
        }

        std::string norm_name = utils::normalize_name(name);
        std::string norm_description = utils::normalize_name(description);

        parser::FileFormat fileFormat;
        if (!parser::FileFormatBuilder::build(format, fileFormat, msg)) {
          result.add_error(msg);
          return false;
        }

        std::string id_clone;
        if (!dba::clone_dataset(id, name, norm_name, description, norm_description, fileFormat, extra_metadata, id_clone, msg)) {
          result.add_error(msg);
          return false;
        }

        result.add_string(id_clone);

        return true;
      }

    } cloneDatasetCommandCommand;
  }
}
