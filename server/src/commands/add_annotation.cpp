//
//  add_annotation.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 20.08.13.
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

#include <string>
#include <sstream>

#include "../datatypes/user.hpp"

#include "../dba/dba.hpp"
#include "../dba/exists.hpp"
#include "../dba/insert.hpp"

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

    class AddAnnotationCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::ANNOTATIONS, "Add a custom annotation of genomic regions such as, for instance, promoters, transcription factor binding sites, or genes to DeepBlue. Annotations are a set genomic regions such as, for instance, promoters, transcription factor binding sites, or genes to DeepBlue.");
      }

      static Parameters parameters_()
      {
        return {
          Parameter("name", serialize::STRING, "annotation name"),
          parameters::Genome,
          Parameter("description", serialize::STRING, "description of the annotation"),
          Parameter("data", serialize::DATASTRING, "the BED formatted data"),
          Parameter("format", serialize::STRING, "format of the provided data"),
          parameters::AdditionalExtraMetadata,
          parameters::UserKey
        };
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
        datatypes::User user;

        if (!check_permissions(user_key, datatypes::INCLUDE_ANNOTATIONS, user, msg )) {
          result.add_error(msg);
          return false;
        }

        datatypes::Metadata extra_metadata;
        if (!read_metadata(parameters[5], extra_metadata, msg)) {
          result.add_error(msg);
          return false;
        }

        std::string norm_name = utils::normalize_annotation_name(name);
        std::string norm_description = utils::normalize_name(description);
        std::string norm_genome = utils::normalize_name(genome);

        if (dba::exists::annotation(norm_name, norm_genome)) {
          std::string s = "The annotation name " + name + " is already being used for the genome " + genome;
          result.add_error(s);
          return false;
        }

        if (!dba::exists::genome(norm_genome)) {
          result.add_error("Invalid genome: " + genome);
          return false;
        }

        bool ret;
        std::string id;

        std::unique_ptr<std::istream> _input = std::unique_ptr<std::istream>(new std::stringstream(data));

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

          ret = dba::insert_annotation(user, name, norm_name, genome, norm_genome, description, norm_description, extra_metadata,
                                       ip, wig, id, msg);
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

          parser::Parser parser(std::move(_input), fileFormat);
          if (!parser.check_format(msg)) {
            result.add_error(msg);
            return false;
          }

          parser::ChromosomeRegionsMap map_regions;

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

          ret = dba::insert_annotation(user, name, norm_name, genome, norm_genome, description, norm_description, extra_metadata,
                                       ip, map_regions, fileFormat, id, msg);
        }

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
