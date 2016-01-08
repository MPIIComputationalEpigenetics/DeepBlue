//
//  wig_parser.hpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 20.01.14.
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

#ifndef WIG_PARSER_HPP
#define WIG_PARSER_HPP

#include <memory>
#include <sstream>
#include <list>

#include <memory>

#include "wig.hpp"

namespace epidb {
  namespace parser {

    static const size_t BLOCK_SIZE = 8000;

    class WIGParser {
    protected:
      size_t actual_line_;
      std::unique_ptr<std::istream> input_;
      std::map<std::string, std::string> info_;
      bool declare_track_;
      TrackPtr actual_track;
      std::map<std::string, std::shared_ptr<boost::icl::interval_set<int> > > map_overlap_counter;
      bool read_track(const std::string &line, std::map<std::string, std::string> &info, std::string &msg);
      bool read_parameters(const std::vector<std::string> &strs, std::map<std::string, std::string> &params, std::string &msg);
      bool read_format(std::string &line, TrackPtr &track, std::string &msg);
      bool check_feature(const std::string &chromosome, const size_t start, const size_t span, const size_t &line, std::string &msg);
      void check_block_size(WigPtr &wig);
      bool get_line(std::string line, std::string &msg);

      const std::string line_str()
      {
        return utils::integer_to_string(actual_line_);
      }

    public:
      WIGParser(std::unique_ptr<std::istream> &&input);
      bool get(WigPtr &wig, std::string &msg);
      size_t actual_line();
      bool eof();
    };
  }
}

#endif /* defined(WIG_PARSER_HPP) */