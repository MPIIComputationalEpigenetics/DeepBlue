//
//  salmon_parser.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 24.01.17.
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

#include "salmon_parser.hpp"
#include "salmon.hpp"

#include "../interfaces/serializable.hpp"

#include "../types.hpp"

namespace epidb {
  namespace parser {

    SalmonParser::SalmonParser(std::unique_ptr<std::istream> &&input) :
      IGeneExpressionParser(std::move(input))
    {}

    bool SalmonParser::parse(ISerializableFilePtr& file, std::string &msg)
    {
      std::shared_ptr<TPMFile> tpm_file = std::make_shared<TPMFile>();

      strtk::for_each_line_conditional(*input_, [&](const std::string & line) -> bool {
        actual_line_++;
        if (line.empty() || line[0] == '#')
        {
          return true;
        }

        std::string tracking_id;
        double length;
        double effective_length;
        double tpm;
        double num_reads;

        if (!strtk::parse(line, "\t", tracking_id, length, effective_length, tpm, num_reads))
        {
          // The first line can be the header.
          if (actual_line_ == 1) {
            return true;
          }
          msg = "Failed to parse line " + line_str() + ": " + line;
          return false;
        }

        tpm_file->add_row(tracking_id, length, effective_length, tpm, num_reads);

        return true;
      });

      // Verify error
      if (!msg.empty()) {
        return false;
      }

      file = tpm_file;
      return true;
    }
  }
}
