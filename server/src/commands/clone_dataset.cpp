//
//  clone_dataset.cpp
//  epidb
//
//  Created by Felipe Albrecht on 01.10.2014.
//  Copyright (c) 2016 Max Planck Institute for Informatics. All rights reserved.
//

#include <string>

#include "../dba/clone.hpp"
#include "../dba/dba.hpp"
#include "../dba/exists.hpp"
#include "../dba/list.hpp"

#include "../datatypes/user.hpp"

#include "../engine/commands.hpp"

#include "../extras/serialize.hpp"

#include "../errors.hpp"

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
          Parameter("new_epigenetic_mark", serialize::STRING, "New epigenetic mark"),
          Parameter("new_sample", serialize::STRING, "New sample ID"),
          Parameter("new_technique", serialize::STRING, "New technique"),
          Parameter("new_project", serialize::STRING, "New project"),
          Parameter("description", serialize::STRING, "description of the experiment - empty to copy from the cloned dataset"),
          Parameter("format", serialize::STRING, "format of the provided data - empty to copy from the cloned dataset"),
          Parameter("extra_metadata", serialize::MAP, "additional metadata - empty to copy from the cloned dataset"),
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 10);
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
        const std::string epigenetic_mark = parameters[2]->as_string();
        const std::string sample = parameters[3]->as_string();
        const std::string technique = parameters[4]->as_string();
        const std::string project = parameters[5]->as_string();
        const std::string description = parameters[6]->as_string();;
        const std::string format = parameters[7]->as_string();
        const std::string user_key = parameters[9]->as_string();

        std::string msg;
        datatypes::User user;

        if (!check_permissions(user_key, datatypes::INCLUDE_EXPERIMENTS, user, msg )) {
          result.add_error(msg);
          return false;
        }

        if (name.empty()) {
          result.add_error("Experiment name can not be empty.");
          return false;
        }

        datatypes::Metadata extra_metadata;
        if (!read_metadata(parameters[8], extra_metadata, msg)) {
          result.add_error(msg);
          return false;
        }

        std::string norm_name = utils::normalize_name(name);
        std::string norm_epigenetic_mark = utils::normalize_name(epigenetic_mark);
        std::string norm_technique = utils::normalize_name(technique);
        std::string norm_project = utils::normalize_name(project);
        std::string norm_description = utils::normalize_name(description);

        if (dba::exists::experiment(norm_name)) {
          std::string s = Error::m(ERR_DUPLICATED_EXPERIMENT_NAME, name);
          result.add_error(s);
          return false;
        }

        if (!epigenetic_mark.empty()) {
          if (!dba::exists::epigenetic_mark(norm_epigenetic_mark)) {
            std::vector<utils::IdName> names;
            if (!dba::list::similar_epigenetic_marks(epigenetic_mark, user_key, names, msg)) {
              result.add_error(msg);
              return false;
            }
            std::stringstream ss;
            ss << "Invalid epigenetic mark: ";
            ss << epigenetic_mark;
            ss << ".";
            if (names.size() > 0) {
              ss << " It is suggested the following names: ";
              ss << utils::vector_to_string(names);
            }
            result.add_error(ss.str());
            return false;
          }
        }

        if (!technique.empty()) {
          if (!dba::exists::technique(norm_technique)) {
            std::vector<utils::IdName> names;
            if (!dba::list::similar_techniques(technique, user_key, names, msg)) {
              result.add_error(msg);
              return false;
            }
            std::stringstream ss;
            ss << "Invalid technique name: ";
            ss << technique;
            ss << ".";
            if (names.size() > 0) {
              ss << " The following names are suggested: ";
              ss << utils::vector_to_string(names);
            }
            result.add_error(ss.str());
            return false;
          }
        }

        // TODO: check the sample

        if (!project.empty()) {
          if (!dba::exists::project(norm_project)) {
            std::vector<utils::IdName> names;
            if (!dba::list::similar_projects(project, user_key, names, msg)) {
              result.add_error(msg);
              return false;
            }
            std::stringstream ss;
            ss << "Invalid project name. ";
            ss << project;
            ss << ".";
            if (names.size() > 0) {
              ss << " The following names are suggested: ";
              ss << utils::vector_to_string(names);
            }
            result.add_error(ss.str());
            return false;
          }
        }

        std::string id_clone;
        if (!dba::clone_dataset(id, name, norm_name, epigenetic_mark, norm_epigenetic_mark, sample, technique, norm_technique, project, norm_project, description, norm_description, format, extra_metadata, user_key, ip, id_clone, msg)) {
          result.add_error(msg);
          return false;
        }

        result.add_string(id_clone);

        return true;
      }

    } cloneDatasetCommandCommand;
  }
}
