//
//  upload_chromosome.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Fabian Reinartz on 24.03.14.
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

#include "../dba/dba.hpp"
#include "../dba/exists.hpp"
#include "../dba/genomes.hpp"
#include "../dba/helpers.hpp"

#include "../engine/commands.hpp"

#include "../extras/utils.hpp"

#include "../parser/fasta_parser.hpp"
#include "../parser/genome_data.hpp"

#include "../errors.hpp"

namespace epidb {
  namespace command {

    class UploadChromosomeCommand: public Command {

    private:
      static CommandDescription desc_()
      {
        return CommandDescription(categories::GENOMES, "Upload the DNA sequence of a chromosome.");
      }

      static Parameters parameters_()
      {
        Parameter p[] = {
          parameters::Genome,
          Parameter("chromosome", serialize::STRING, "chromosome name"),
          Parameter("data", serialize::DATASTRING, "chromosome sequence data"),
          parameters::UserKey
        };
        Parameters params(&p[0], &p[0] + 4);
        return params;
      }

      static Parameters results_()
      {
        Parameter p[] = {};
        Parameters results(&p[0], &p[0]);
        return results;
      }

    public:
      UploadChromosomeCommand() : Command("upload_chromosome", parameters_(), results_(), desc_()) {}

      virtual bool run(const std::string &ip,
                       const serialize::Parameters &parameters, serialize::Parameters &result) const
      {
        const std::string genome = parameters[0]->as_string();
        const std::string chromosome = parameters[1]->as_string();
        const std::string data = parameters[2]->as_string();
        const std::string user_key = parameters[3]->as_string();

        std::string msg;
        if (!Command::checks(user_key, msg)) {
          result.add_error(msg);
          return false;
        }

        std::string norm_genome = utils::normalize_name(genome);
        if (!dba::exists::genome(norm_genome)) {
          result.add_error("Invalid genome '" + genome + "'");
          return false;
        }

        dba::genomes::GenomeInfoPtr genome_info;
        if (!dba::genomes::get_genome_info(genome, genome_info, msg)) {
          result.add_error("Could not get the genome " + msg + " information.");
          return false;
        }

        std::string clear_data;

        if (!parser::fasta::clean_up(data, clear_data, msg)) {
          result.add_error(msg);
          return false;
        }

        dba::genomes::ChromosomeInfo chromosome_info;
        if (!genome_info->get_chromosome(chromosome, chromosome_info, msg)) {
          result.add_error(msg);
          return false;
        }

        if (clear_data.length() != chromosome_info.size) {
          std::stringstream ss;
          ss << "Uploaded sequence does not have the correct size.";
          ss << " It is expected a chromosome with ";
          ss << chromosome_info.size;
          ss << " but the given chromosome has ";
          ss << clear_data.length();
          ss << " bases.";
          result.add_error(ss.str());
          return false;
        }

        bool ret = dba::add_chromosome_sequence(genome, norm_genome, chromosome, clear_data, user_key, msg);

        if (!ret) {
          result.add_error(msg);
        }
        return ret;
      }
    } uploadChromosomeCommand;
  }
}
