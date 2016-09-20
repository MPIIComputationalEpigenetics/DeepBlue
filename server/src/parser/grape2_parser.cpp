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

#include <boost/algorithm/string.hpp>

#include <strtk.hpp>

#include "grape2.hpp"
#include "grape2_parser.hpp"

#include "../extras/utils.hpp"

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

        std::vector<std::string> strs;
        boost::split(strs, line, boost::is_any_of("\t"));

        // TODO: check list size
        gene_id = strs[0];
        transcript_ids = strs[1];

        // If it is the first line and it is a header.
        if ((actual_line_ == 1) && (strs[2].compare(0, 6, "length") == 0)) {
          return true;
        }

        if (!utils::string_to_double(strs[2], length))
        {
          msg = "Error at line " + line_str() + " . The field 'length' with the value '"  + strs[2] + "'";
          return false;
        }

        if (!utils::string_to_double(strs[3], effective_length))
        {
          msg = "Error at line " + line_str() + " . The field 'effective_length' with the value '"  + strs[3] + "'";
          return false;
        }

        if (!utils::string_to_double(strs[4], expected_count))
        {
          msg = "Error at line " + line_str() + " . The field 'expected_count' with the value '"  + strs[4] + "'";
          return false;
        }

        if (!utils::string_to_double(strs[5], TPM))
        {
          msg = "Error at line " + line_str() + " . The field 'TPM' with the value '"  + strs[5] + "'";
          return false;
        }

        if (!utils::string_to_double(strs[6], FPKM))
        {
          msg = "Error at line " + line_str() + " . The field 'FPKM' with the value '"  + strs[6] + "'";
          return false;
        }

        if (!utils::string_to_double(strs[7], posterior_mean_count))
        {
          msg = "Error at line " + line_str() + " . The field 'posterior_mean_count' with the value '"  + strs[7] + "'";
          return false;
        }

        if (!utils::string_to_double(strs[8], posterior_standard_deviation_of_count))
        {
          msg = "Error at line " + line_str() + " . The field 'posterior_standard_deviation_of_count' with the value '"  + strs[8] + "'";
          return false;
        }

        if (!utils::string_to_double(strs[9], pme_TPM))
        {
          msg = "Error at line " + line_str() + " . The field 'pme_TPM' with the value '"  + strs[9] + "'";
          return false;
        }

        if (!utils::string_to_double(strs[10], pme_FPKM))
        {
          msg = "Error at line " + line_str() + " . The field 'pme_FPKM' with the value '"  + strs[10] + "'";
          return false;
        }

        if (!utils::string_to_double(strs[11], TPM_ci_lower_bound))
        {
          msg = "Error at line " + line_str() + " . The field 'TPM_ci_lower_bound' with the value '"  + strs[11] + "'";
          return false;
        }

        if (!utils::string_to_double(strs[12], TPM_ci_upper_bound))
        {
          msg = "Error at line " + line_str() + " . The field 'TPM_ci_upper_bound' with the value '"  + strs[12] + "'";
          return false;
        }

        if (!utils::string_to_double(strs[13], FPKM_ci_lower_bound))
        {
          msg = "Error at line " + line_str() + " . The field 'FPKM_ci_lower_bound' with the value '"  + strs[13] + "'";
          return false;
        }

        if (!utils::string_to_double(strs[14], FPKM_ci_upper_bound))
        {
          msg = "Error at line " + line_str() + " . The field 'FPKM_ci_upper_bound' with the value '"  + strs[14] + "'";
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
