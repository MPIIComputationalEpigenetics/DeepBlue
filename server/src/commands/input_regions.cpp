//
//  input_regions.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 14.07.15.
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

#include "../dba/dba.hpp"
#include "../dba/insert.hpp"
#include "../dba/queries.hpp"
#include "../dba/users.hpp"

#include "../engine/commands.hpp"

#include "../extras/utils.hpp"
#include "../extras/serialize.hpp"

#include "../parser/parser_factory.hpp"

#include "../errors.hpp"

namespace epidb {
  namespace command {

    class InputRegionsCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::OPERATIONS, "Upload a set of genomic regions that can be accessed through a query ID. An interesting use case for this command is to upload a set of custom regions for intersecting with genomic regions in DeepBlue to specifically select regions of interest.");
      }

      static Parameters parameters_()
      {
        return {
          parameters::Genome,
          Parameter("region_set", serialize::DATASTRING, "Regions in CHROMOSOME\tSTART\tEND format"),
          parameters::UserKey
        };
      }

      static Parameters results_()
      {
        return {
          Parameter("id", serialize::STRING, "query id")
        };
      }

    public:
      InputRegionsCommand() : Command("input_regions", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        const std::string genome = parameters[0]->as_string();
        const std::string data = parameters[1]->as_string();
        const std::string user_key = parameters[2]->as_string();

        std::string msg;
        datatypes::User user;

        if (!check_permissions(user_key, datatypes::GET_DATA, user, msg )) {
          result.add_error(msg);
          return false;
        }

        if (data.empty()) {
          result.add_error("The region set can not be empty.");
          return false;
        }

        std::string norm_genome = utils::normalize_name(genome);

        parser::FileFormat fileFormat;
        parser::ChromosomeRegionsMap map_regions;

        if (!parser::FileFormatBuilder::build("", fileFormat, msg)) {
          result.add_error(msg);
          return false;
        }

        std::unique_ptr<std::istream> _input = std::unique_ptr<std::istream>(new std::stringstream(data));
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

        int dataset_id;
        if (!dba::insert_query_region_set(user, genome, norm_genome,ip, map_regions, fileFormat, dataset_id, msg)) {
          result.add_error(msg);
          return false;
        }

        std::vector<std::string> chromosomes = map_regions.chromosomes();

        mongo::BSONObjBuilder args_builder;
        args_builder.append("dataset_id", dataset_id);
        args_builder.append("genome", genome);
        args_builder.append("norm_genome", norm_genome);
        args_builder.append("chromosomes", utils::build_array(chromosomes));

        std::string query_id;
        if (!dba::query::store_query(user, "input_regions", args_builder.obj(), query_id, msg)) {
          result.add_error(msg);
          return false;
        }

        result.add_string(query_id);
        return true;
      }
    } inputRegions;
  }
}

