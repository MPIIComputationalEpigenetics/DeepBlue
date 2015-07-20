//
//  input_regions.cpp
//  epidb
//
//  Created by Felipe Albrecht on 14.07.15.
//  Copyright (c) 2013,2014,2015 Max Planck Institute for Computer Science. All rights reserved.
//

#include <fstream>
#include <iostream>
#include <string>
#include <sstream>

#include <boost/foreach.hpp>

#include "../dba/dba.hpp"
#include "../dba/helpers.hpp"
#include "../dba/insert.hpp"
#include "../dba/queries.hpp"

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
        return CommandDescription(categories::OPERATIONS, "Include a region set that will be used by the follow ups operations.");
      }

      static Parameters parameters_()
      {
        Parameter p[] = {
          parameters::Genome,
          Parameter("region_set", serialize::DATASTRING, "Regions in CHROMOSOME\tSTART\tEND format"),
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 3);
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
        if (!dba::insert_query_region_set(genome, norm_genome, user_key, ip, map_regions, fileFormat, dataset_id, msg)) {
          result.add_error(msg);
        }

        std::vector<std::string> chromosomes = map_regions.chromosomes();

        mongo::BSONObjBuilder args_builder;
        args_builder.append("dataset_id", dataset_id);
        args_builder.append("genome", genome);
        args_builder.append("norm_genome", norm_genome);
        args_builder.append("chromosomes", dba::helpers::build_array(chromosomes));

        std::string query_id;
        if (!dba::query::store_query("input_regions", args_builder.obj(), user_key, query_id, msg)) {
          result.add_error(msg);
          return false;
        }

        result.add_string(query_id);
        return true;
      }
    } inputRegions;
  }
}

