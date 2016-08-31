//
//  grape2_parser.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 08.08.16.
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

#include <ctime>
#include <limits>
#include <string>

#include <strtk.hpp>

#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>

#include "grape2.hpp"
#include "grape2_parser.hpp"

#include "../types.hpp"


namespace epidb {
  namespace parser {

    Grape2Parser::Grape2Parser(std::unique_ptr<std::istream> &&input) :
      IGeneExpressionParser(std::move(input))
    {}

    bool Grape2Parser::parse(ISerializableFilePtr& file, std::string &msg)
    {
      std::shared_ptr<Grape2File> grape2_file = std::make_shared<Grape2File>();

      strtk::for_each_line_conditional(*input_, [&](const std::string & line) -> bool {
        actual_line_++;
        if (line.empty() || line[0] == '#')
        {
          return true;
        }

        std::string gene_id;
        std::string transcript_ids;
        double length;
        double effective_length;
        double expected_count;
        double TPM;
        double FPKM;
        double posterior_mean_count;
        double posterior_standard_deviation_of_count;
        double pme_TPM;
        double pme_FPKM;
        double TPM_ci_lower_bound;
        double TPM_ci_upper_bound;
        double FPKM_ci_lower_bound;
        double FPKM_ci_upper_bound;

        if (
          !strtk::parse(line, "\t", gene_id, transcript_ids, length, effective_length, expected_count, TPM, FPKM,
        posterior_mean_count, posterior_standard_deviation_of_count, pme_TPM, pme_FPKM)
          ||
          !strtk::parse(line, "\t",
        TPM_ci_lower_bound, TPM_ci_upper_bound, FPKM_ci_lower_bound, FPKM_ci_upper_bound)
        )
        {
          // The first line can be the header.
          if (actual_line_ == 1) {
            return true;
          }
          msg = "Failed to parse line : " + line_str() + " " + line;
          return false;
        }

        grape2_file->add_row(gene_id, transcript_ids, length, effective_length, expected_count,
                             TPM, FPKM,
                             posterior_mean_count, posterior_standard_deviation_of_count,
                             pme_TPM, pme_FPKM,
                             TPM_ci_lower_bound, TPM_ci_upper_bound,
                             FPKM_ci_lower_bound, FPKM_ci_upper_bound);
        return true;
      });

      // Verify error
      if (!msg.empty()) {
        return false;
      }

      file = grape2_file;
      return true;
    }
  }
}
