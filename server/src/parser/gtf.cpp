//
//  gtf.cpp
//  epidb
//
//  Created by Felipe Albrecht on 07.09.15.
//  Copyright (c) 2015 Max Planck Institute for Computer Science. All rights reserved.
//

#include "gtf.hpp"
#include "../types.hpp"

namespace epidb {
  namespace parser {

    GTFRow::GTFRow(const std::string &seqname, const std::string &source, const std::string &feature,
                   Position start, Position end, Score score,
                   char strand, char frame,
                   const std::string gene_id, const std::string transcript_id, Attributes&& attributes):
      _seqname(seqname),
      _source(source),
      _feature(feature),
      _start(start),
      _end(end),
      _score(score),
      _frame(frame),
      _gene_id(gene_id),
      _transcript_id(transcript_id),
      _attributes(attributes)
    { }

    void GTFFile::add_row(GTFRow&& row)
    {
      _content.push_back(row);
    }

    GTFContent::const_iterator GTFFile::rows_iterator() const
    {
      return _content.begin();
    }

    GTFContent::const_iterator GTFFile::rows_iterator_end() const
    {
      return _content.end();
    }

    size_t GTFFile::size() const
    {
      return _content.size();
    }

  }
}