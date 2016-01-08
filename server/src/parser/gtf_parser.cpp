//
//  gtf_parser.cpp
//  epidb
//
//  Created by Felipe Albrecht on 29.04.14.
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

#include "gtf_parser.hpp"
#include "gtf.hpp"

#include "../types.hpp"

namespace epidb {
  namespace parser {

    GTFParser::GTFParser(std::unique_ptr<std::istream> &&input) :
      actual_line_(0),
      input_(std::move(input))
    {}

    struct gtf_line {
      std::string seqname;
      std::string source;
      std::string feature;
      Position start;
      Position end;
      std::string score;
      std::string strand;
      std::string frame;               // frame - One of '0', '1' or '2'. '0' indicates that the first base of the feature is the first base of a codon, '1' that the second base is the first base of a codon, and so on..
      std::string s_attributes; // attribute - A semicolon-separated list of tag-value pairs, providing additional information about each feature.

    };

    bool GTFParser::parse_attributes(const std::string& line, const std::string& s_attributes, GTFRow::Attributes& attributes, std::string& msg)
    {
      // gene_id "ENSG00000223972"; gene_name "DDX11L1"; gene_source "havana"; gene_biotype "transcribed_unprocessed_pseudogene";
      std::string separator1("");//dont let quoted arguments escape themselves
      std::string separator2(";");//split on semilcolon
      std::string separator3("\"\'");//let it have quoted arguments

      boost::escaped_list_separator<char> els(separator1, separator2, separator3);
      boost::tokenizer<boost::escaped_list_separator<char> > tok(s_attributes, els);

      attributes["gene_id"] = "";
      for (boost::tokenizer<boost::escaped_list_separator<char> >::iterator beg = tok.begin(); beg != tok.end(); ++beg) {
        std::string pair = *beg;
        boost::trim(pair);
        if (pair.empty()) {
          continue;
        }
        size_t pos = pair.find_first_of(" ");
        if (pos == std::string::npos) {
          msg = "The track seems to have an invalid attribute value: '" + *beg + "' in the line: " + line;
          return false;
        }
        std::string key = pair.substr(0, pos);
        std::string value = pair.substr(pos + 1);
        attributes[key] = value;
      }

      return true;
    }

    bool GTFParser::get(parser::GTFPtr &gtf, std::string &msg)
    {
      gtf = std::shared_ptr<GTFFile>(new GTFFile());

      strtk::for_each_line_conditional(*input_, [&](const std::string & line) -> bool {

        actual_line_++;
        if (line.empty() || line[0] == '#')
        {
          return true;
        }

        gtf_line row;
        if (!strtk::parse(line, "\t", row.seqname, row.source, row.feature, row.start, row.end, row.score, row.strand, row.frame, row.s_attributes))
        {
          msg = "Failed to parse line : " + line_str() + " " + line;
          return false;
        }

        // Ignore if the feature is not gene
        if (row.feature != "gene") {
          return true;
        }

        GTFRow::Attributes attributes;
        if (!parse_attributes(line, row.s_attributes, attributes, msg))
        {
          return false;
        }

        Score score;
        if (row.score == ".") {
          score = std::numeric_limits<Score>::min();
        } else {
          if (!utils::string_to_score(row.score, score)) {
            msg = "The score value " + row.score + " is invalid. Line: " + line_str();
          }
        }

        gtf->add_row(row.seqname, row.source, row.feature, row.start, row.end, score, row.strand, row.frame, attributes);

        return true;
      });

      // Verify error
      if (!msg.empty()) {
        return false;
      }

      return true;
    }
  }
}
