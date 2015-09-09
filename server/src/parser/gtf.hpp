//
//  gtf.hpp
//  epidb
//
//  Created by Felipe Albrecht on 04.09.15.
//  Copyright (c) 2015 Max Planck Institute for Computer Science. All rights
//  reserved.
//

#ifndef GTF_HPP
#define GTF_HPP

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <boost/utility.hpp>

#include "../extras/utils.hpp"
#include "../types.hpp"

namespace epidb {
  namespace parser {

    class GTFRow {
    public:
      typedef std::unordered_map<std::string, std::string> Attributes;

    private:
      std::string _seqname;
      std::string _source;
      std::string _feature;
      Position _start;
      Position _end;
      Score _score;
      char _strand;
      char _frame;  // frame - One of '0', '1' or '2'. '0' indicates that the first base of the feature is the first base of a codon, '1' that the second base is the first base of a codon, and so on..
      Attributes _attributes;

    public:
      GTFRow(const std::string &seqname, const std::string &source, const std::string &feature,
             Position start, Position end, Score score,
             char strand, char frame, const Attributes& attributes);

      std::string chromosome() const { return _seqname; }
      Position start() const { return _start; }
      Position end() const { return _end; }
      Score score() const { return _score; }
      Score strand() const { return _strand; }
      Score frame() const { return _frame; }
      Attributes attributes() const { return _attributes; }
    };

    typedef std::vector<GTFRow> GTFContent;

    class GTFFile : boost::noncopyable {
    private:
      GTFContent _content;

    public:
      const GTFContent& rows() const;
      void add_row(const std::string &seqname, const std::string &source, const std::string &feature,
                   Position start, Position end, Score score,
                   char strand, char frame, const GTFRow::Attributes& attributes);
      size_t size() const;
    };

    typedef std::shared_ptr<GTFFile> GTFPtr;
  }
}

#endif /* defined(WIG_HPP) */
