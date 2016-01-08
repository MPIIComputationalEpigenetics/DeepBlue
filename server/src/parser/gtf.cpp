//
//  gtf.cpp
//  epidb
//
//  Created by Felipe Albrecht on 07.09.15.
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

#include "gtf.hpp"
#include "../types.hpp"

namespace epidb {
  namespace parser {

    GTFRow::GTFRow(const std::string &seqname, const std::string &source, const std::string &feature,
                   Position start, Position end, Score score,
                   std::string strand, std::string frame, const Attributes& attributes):
      _seqname(seqname),
      _source(source),
      _feature(feature),
      _start(start),
      _end(end),
      _score(score),
      _strand(strand),
      _frame(frame),
      _attributes(attributes)
    { }

    void GTFFile::add_row(const std::string &seqname, const std::string &source, const std::string &feature,
                   Position start, Position end, Score score,
                   std::string strand, std::string frame, const GTFRow::Attributes& attributes)
    {
      _content.emplace_back(seqname, source, feature, start, end, score, strand, frame, attributes);
    }

    const GTFContent& GTFFile::rows() const
    {
      return _content;
    }

    size_t GTFFile::size() const
    {
      return _content.size();
    }

  }
}