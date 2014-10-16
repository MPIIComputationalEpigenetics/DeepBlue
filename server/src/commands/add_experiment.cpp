//
//  insert.cpp
//  epidb
//
//  Created by Felipe Albrecht on 29.05.13.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include <fstream>
#include <iostream>
#include <string>
#include <sstream>

#include <boost/foreach.hpp>

#include "../dba/dba.hpp"
#include "../dba/insert.hpp"
#include "../dba/list.hpp"

#include "../engine/commands.hpp"

#include "../extras/utils.hpp"
#include "../extras/serialize.hpp"

#include "../parser/field_type.hpp"
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
        return CommandDescription(categories::EXPERIMENTS, "Inserts a new experiment with the given parameters.");
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
          Parameter("extra_metadata", serialize::MAP, "additional metadata"),
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

        bool ok = false;
        std::string msg;

        if (!dba::check_user(user_key, ok, msg)) {
          result.add_error(msg);
          return false;
        }
        if (!ok) {
          std::string s = Error::m(ERR_INVALID_USER_KEY);
          result.add_error(s);
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

        if (description.size() > 10000) {
          result.add_error("Description is too long.");
          return false;
        }

        if (!dba::check_experiment_name(name, norm_name, user_key, ok, msg)) {
          result.add_error(msg);
          return false;
        }
        if (!ok) {
          std::string s = "The experiment name " + name + " is already being used.";
          result.add_error(s);
          return false;
        }

        if (!dba::check_genome(norm_genome, ok, msg)) {
          result.add_error(msg);
          return false;
        }
        if (!ok) {
          result.add_error("Invalid genome '" + genome + "'");
          return false;
        }

        if (!dba::check_epigenetic_mark(norm_epigenetic_mark, ok, msg)) {
          result.add_error(msg);
          return false;
        }
        if (!ok) {
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

        if (!dba::check_technique(norm_technique, ok, msg)) {
          result.add_error(msg);
          return false;
        }
        if (!ok) {
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

        if (!dba::check_project(norm_project, ok, msg)) {
          result.add_error(msg);
          return false;
        }
        if (!ok) {
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

        if (format == "wig" || format == "bedgraph") {
          parser::WigPtr wig;
          std::unique_ptr<std::istream> _input;
          if (extra_metadata.find("__local_file__") != extra_metadata.end()) {
            std::string& file_name = extra_metadata["__local_file__"];
            _input = std::unique_ptr<std::istream>(new std::ifstream(file_name.c_str()));
            if (!_input->good()) {
              result.add_error("File " + file_name + " does not exist or it is not accessible.");
              return false;
            }
          } else {
            _input = std::unique_ptr<std::istream>(new std::stringstream(data));
          }

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
          if (!parser::FileFormatBuilder::build(format, fileFormat, msg)) {
            result.add_error(msg);
            return false;
          }

          parser::Parser parser(data, fileFormat);
          std::vector<parser::Tokens> bed_file_tokenized;
          while (!parser.eof()) {
            parser::Tokens tokens;

            if (parser.parse_line(tokens)) {
              if (parser.check_length(tokens)) {
                bed_file_tokenized.push_back(tokens);
              } else {
                std::stringstream m;
                m << "Error while reading the BED file. Line: ";
                m << parser.actual_line();
                m << ". - '";
                m << parser.actual_line_content();
                m << "'. The number of tokens (" ;
                m << tokens.size() ;
                m << ") is different from the format size (" ;
                m << parser.count_fields();
                m << ") - ";
                m << fileFormat.format();
                result.add_error(m.str());
                return false;
              }
            }
          }

          std::string id;
          bool ret = dba::insert_experiment(name, norm_name, genome, norm_genome, epigenetic_mark, norm_epigenetic_mark, sample,
                                            technique, norm_technique, project, norm_project, description, norm_description,
                                            extra_metadata, user_key, ip, bed_file_tokenized, fileFormat, id, msg);
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
