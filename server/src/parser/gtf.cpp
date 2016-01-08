//
//  gtf.cpp
//  epidb
//
//  Created by Felipe Albrecht on 07.09.15.
//  Copyright (c) 2016 Max Planck Institute for Informatics. All rights reserved.
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