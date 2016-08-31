//
//  cufflinks_parser.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 29.06.16.
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

#include <strtk.hpp>

#include "cufflinks_parser.hpp"
#include "fpkm.hpp"

#include "../interfaces/serializable.hpp"

#include "../types.hpp"

namespace epidb {
  namespace parser {

    CufflinksParser::CufflinksParser(std::unique_ptr<std::istream> &&input) :
      IGeneExpressionParser(std::move(input))
    {}

    bool CufflinksParser::parse(ISerializableFilePtr& file, std::string &msg)
    {
      std::shared_ptr<FPKMFile> fpkm_file = std::make_shared<FPKMFile>();

      strtk::for_each_line_conditional(*input_, [&](const std::string & line) -> bool {
        actual_line_++;
        if (line.empty() || line[0] == '#')
        {
          return true;
        }

        std::string tracking_id; // 0
        std::string gene_id; // 3
        std::string gene_short_name; // 4
        Score FPKM; // 9
        Score FPKM_conf_lo; // 10
        Score FPKM_conf_hi; // 11
        std::string FPKM_status; // 12

        if (!strtk::parse_columns(line, "\t", strtk::column_list(0, 3, 4, 9, 10, 11, 12), tracking_id, gene_id, gene_short_name, FPKM, FPKM_conf_lo, FPKM_conf_hi, FPKM_status))
        {
          // The first line can be the header.
          if (actual_line_ == 1) {
            return true;
          }
          msg = "Failed to parse line : " + line_str() + " " + line;
          return false;
        }

        fpkm_file->add_row(tracking_id, gene_id, gene_short_name, FPKM, FPKM_conf_lo, FPKM_conf_hi, FPKM_status);

        return true;
      });

      // Verify error
      if (!msg.empty()) {
        return false;
      }

      file = fpkm_file;
      return true;
    }
  }
}
