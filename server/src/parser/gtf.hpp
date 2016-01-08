//
//  gtf.hpp
//  DeepBlue Epigenomic Data Server

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
      std::string _strand;
      std::string _frame;  // frame - One of '0', '1' or '2'. '0' indicates that the first base of the feature is the first base of a codon, '1' that the second base is the first base of a codon, and so on..
      Attributes _attributes;

    public:
      GTFRow(const std::string &seqname, const std::string &source, const std::string &feature,
             Position start, Position end, Score score,
             std::string strand, std::string frame, const Attributes& attributes);

      const std::string& seqname() const { return _seqname; }
      const std::string& source() const { return _source; }
      const std::string& feature() const { return _feature; }
      const Position start() const { return _start; }
      const Position end() const { return _end; }
      const Score score() const { return _score; }
      const std::string strand() const { return _strand; }
      const std::string frame() const { return _frame; }
      const Attributes& attributes() const { return _attributes; }
    };

    typedef std::vector<GTFRow> GTFContent;

    class GTFFile : boost::noncopyable {
    private:
      GTFContent _content;

    public:
      const GTFContent& rows() const;
      void add_row(const std::string &seqname, const std::string &source, const std::string &feature,
                   Position start, Position end, Score score,
                   std::string strand, std::string frame, const GTFRow::Attributes& attributes);
      size_t size() const;
    };

    typedef std::shared_ptr<GTFFile> GTFPtr;
  }
}

#endif /* defined(WIG_HPP) */
