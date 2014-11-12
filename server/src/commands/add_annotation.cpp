//
//  add_annotation.cpp
//  epidb
//
//  Created by Felipe Albrecht on 20.08.13.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include <string>
#include <sstream>

#include "../dba/dba.hpp"
#include "../dba/insert.hpp"

#include "../engine/commands.hpp"
#include "../extras/utils.hpp"
#include "../extras/serialize.hpp"
#include "../parser/parser_factory.hpp"

#include "../errors.hpp"

namespace epidb {
  namespace command {

    class AddAnnotationCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::ANNOTATIONS, "Inserts a new annotation with the given parameters.");
      }

      static Parameters parameters_()
      {
        Parameter p[] = {
          Parameter("name", serialize::STRING, "annotation name"),
          parameters::Genome,
          Parameter("description", serialize::STRING, "description of the annotation"),
          Parameter("data", serialize::DATASTRING, "the BED formatted data"),
          Parameter("format", serialize::STRING, "format of the provided data"),
          Parameter("extra_metadata", serialize::MAP, "additional metadata"),
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 7);
        return params;
      }

      static Parameters results_()
      {
        Parameter p[] = {
          Parameter("id", serialize::STRING, "id of the newly inserted annotation")
        };
        Parameters results(&p[0], &p[0] + 1);
        return results;
      }

    public:
      AddAnnotationCommand() : Command("add_annotation", parameters_(), results_(), desc_()) {}

      // TODO: Check user
      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        const std::string name = parameters[0]->as_string();
        const std::string genome = parameters[1]->as_string();
        const std::string description = parameters[2]->as_string();
        const std::string data = parameters[3]->as_string();
        const std::string format = parameters[4]->as_string();
        const std::string user_key = parameters[6]->as_string();

        std::string msg;
        if (!Command::checks(user_key, msg)) {
          result.add_error(msg);
          return false;
        }

        datatypes::Metadata extra_metadata;
        if (!read_metadata(parameters[5], extra_metadata, msg)) {
          result.add_error(msg);
          return false;
        }

        std::string norm_name = utils::normalize_annotation_name(name);
        std::string norm_genome = utils::normalize_name(genome);

        bool ok;
        if (!dba::check_annotation(norm_name, norm_genome, ok, msg)) {
          result.add_error(msg);
          return false;
        }
        if (!ok) {
          std::string s = "The annotation name " + name + " is already being used for the genome " + genome;
          result.add_error(s);
          return false;
        }

        bool exists;
        if (!dba::check_genome(norm_genome, exists, msg)) {
          result.add_error(msg);
          return false;
        }
        if (!exists) {
          result.add_error("Invalid genome: " + genome);
          return false;
        }

        parser::FileFormat fileFormat;
        if (!parser::FileFormatBuilder::build(format, fileFormat, msg)) {
          result.add_error(msg);
          return false;
        }

        parser::Parser parser(data, fileFormat);
        if (!parser.check_format(msg)) {
          result.add_error(msg);
          return false;
        }
        std::vector<parser::BedLine> bed_file_tokenized;
        while (!parser.eof()) {
          parser::BedLine bed_line;

          if (!parser.parse_line(bed_line, msg)) {
            std::stringstream m;
            m << "Error while reading the BED file. Line: ";
            m << parser.actual_line();
            m << ". - '";
            m << msg;
            m << "'";
            result.add_error(m.str());
            return false;
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
            bed_file_tokenized.push_back(bed_line);
          }
        }

        std::string norm_description = utils::normalize_name(description);

        std::string id;
        bool ret = dba::insert_annotation(name, norm_name, genome, norm_genome, description, norm_description, extra_metadata,
                                          user_key, ip, bed_file_tokenized, fileFormat, id, msg);

        if (ret) {
          result.add_string(id);
        } else {
          result.add_error(msg);
        }
        return ret;
      }

    } addAnnotationCommand;

  }
}
