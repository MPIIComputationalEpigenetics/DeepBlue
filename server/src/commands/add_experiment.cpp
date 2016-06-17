//
//  insert.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 29.05.13.
//  Copyright (c) 2016 Max Planck Institute for Informatics. All rights reserved.

//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.

//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.

//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#include <fstream>
#include <iostream>
#include <string>
#include <sstream>

#include "../datatypes/user.hpp"

#include "../dba/dba.hpp"
#include "../dba/exists.hpp"
#include "../dba/insert.hpp"
#include "../dba/list.hpp"

#include "../engine/commands.hpp"

#include "../extras/utils.hpp"
#include "../extras/serialize.hpp"

#include "../parser/parser_factory.hpp"
#include "../parser/bedgraph_parser.hpp"
#include "../parser/wig_parser.hpp"
#include "../parser/wig.hpp"

#include "../errors.hpp"

namespace epidb {
  namespace command {

    class AddExperimentCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::EXPERIMENTS, "Include an Experiment in DeepBlue.");
      }

      static Parameters parameters_()
      {
        Parameter p[] = {
          Parameter("name", serialize::STRING, "experiment name"),
          parameters::Genome,
          Parameter("epigenetic_mark", serialize::STRING, "epigenetic mark of the experiment"),
          Parameter("sample", serialize::STRING, "id of the used sample"),
          Parameter("technique", serialize::STRING, "technique used by this experiment"),
          Parameter("project", serialize::STRING, "the project name"),
          Parameter("description", serialize::STRING, "description of the experiment"),
          Parameter("data", serialize::DATASTRING, "the BED formated data"),
          Parameter("format", serialize::STRING, "format of the provided data"),
          parameters::AdditionalExtraMetadata,
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 11);
        return params;
      }

      static Parameters results_()
      {
        Parameter p[] = {
          Parameter("id", serialize::STRING, "id of the newly inserted experiment")
        };
        Parameters results(&p[0], &p[0] + 1);
        return results;
      }

    public:
      AddExperimentCommand() : Command("add_experiment", parameters_(), results_(), desc_()) {}

      // TODO: Check user
      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        const std::string name = parameters[0]->as_string();
        const std::string genome = parameters[1]->as_string();
        const std::string epigenetic_mark = parameters[2]->as_string();
        const std::string sample = parameters[3]->as_string();
        const std::string technique = parameters[4]->as_string();
        const std::string project = parameters[5]->as_string();
        const std::string description = parameters[6]->as_string();
        const std::string data = parameters[7]->as_string();
        const std::string format = parameters[8]->as_string();
        const std::string user_key = parameters[10]->as_string();

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
        if (!read_metadata(parameters[9], extra_metadata, msg)) {
          result.add_error(msg);
          return false;
        }

        std::string norm_name = utils::normalize_name(name);
        std::string norm_genome = utils::normalize_name(genome);
        std::string norm_description = utils::normalize_name(description);
        std::string norm_epigenetic_mark = utils::normalize_epigenetic_mark(epigenetic_mark);
        std::string norm_project = utils::normalize_name(project);
        std::string norm_technique = utils::normalize_name(technique);

        if (description.size() > 2048) {
          result.add_error("Description is too long.");
          return false;
        }

        if (dba::exists::experiment(norm_name)) {
          std::string s = Error::m(ERR_DUPLICATED_EXPERIMENT_NAME, name);
          result.add_error(s);
          return false;
        }

        if (!dba::exists::genome(norm_genome)) {
          result.add_error("Invalid genome '" + genome + "'");
          return false;
        }

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

        // TODO: check the sample

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

        std::unique_ptr<std::istream> _input;
        if (extra_metadata.find("__local_file__") != extra_metadata.end()) {
          std::string &file_name = extra_metadata["__local_file__"];
          _input = std::unique_ptr<std::istream>(new std::ifstream(file_name.c_str()));
          if (!_input->good()) {
            result.add_error("File " + file_name + " does not exist or it is not accessible.");
            return false;
          }
        } else {
          _input = std::unique_ptr<std::istream>(new std::stringstream(data));
        }

        if (format == "wig" || format == "bedgraph") {
          parser::WigPtr wig;
          if (format == "wig") {
            parser::WIGParser wig_parser(std::move(_input));
            if (!wig_parser.get(wig, msg)) {
              result.add_error(msg);
              return false;
            }
          } else {
            parser::BedGraphParser bedgraph_parser(std::move(_input));
            if (!bedgraph_parser.get(wig, msg)) {
              result.add_error(msg);
              return false;
            }
          }

          std::string id;
          bool ret = dba::insert_experiment(name, norm_name, genome, norm_genome, epigenetic_mark, norm_epigenetic_mark, sample,
                                            technique, norm_technique, project, norm_project, description, norm_description,
                                            extra_metadata, user_key, ip, wig, id, msg);
          if (ret) {
            result.add_string(id);
            return true;
          } else {
            result.add_error(msg);
            return false;
          }

        } else {
          parser::FileFormat fileFormat;
          parser::ChromosomeRegionsMap map_regions;

          if (!parser::FileFormatBuilder::build(format, fileFormat, msg)) {
            result.add_error(msg);
            return false;
          }

          parser::Parser parser(std::move(_input), fileFormat);
          if (!parser.check_format(msg)) {
            result.add_error(msg);
            return false;
          }
          while (!parser.eof()) {
            parser::BedLine bed_line;

            if (!parser.parse_line(bed_line, msg)) {
              // Ignore Empty Line Error
              if (msg != "Empty line") {
                std::stringstream m;
                m << "Error while reading the BED file. Line: ";
                m << parser.actual_line();
                m << ". - '";
                m << msg;
                m << "'";
                result.add_error(m.str());
                return false;
              }
            }

            // Ignore empty line
            if (msg == "Empty line") {
              continue;
            }

            if (!parser.check_length(bed_line)) {
              std::stringstream m;
              m << "Error while reading the BED file. Line: ";
              m << parser.actual_line();
              m << ". - '";
              m << parser.actual_line_content();
              m << "'. The number of tokens (" ;
              m << bed_line.size() ;
              m << ") is different from the format size (" ;
              m << parser.count_fields();
              m << ") - ";
              m << fileFormat.format();
              result.add_error(m.str());
              return false;
            } else {
              map_regions.insert(std::move(bed_line));
            }
          }

          map_regions.finish();

          std::string id;
          bool ret = dba::insert_experiment(name, norm_name, genome, norm_genome, epigenetic_mark, norm_epigenetic_mark, sample,
                                            technique, norm_technique, project, norm_project, description, norm_description,
                                            extra_metadata, user_key, ip, map_regions, fileFormat, id, msg);
          if (ret) {
            result.add_string(id);
          } else {
            result.add_error(msg);
          }
          return ret;
        }
      }

    } addExperimentCommand;

  }
}
